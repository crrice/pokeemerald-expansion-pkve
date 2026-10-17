#!/usr/bin/env python3
"""Convert a Verdant Emerald save from expansion 1.14.1 to expansion 1.17.1.

Saves made with any build before the 1.17.1 upgrade (master before 52907a127a)
load wrong on 1.17.1 builds: the Pokedex grew, so part of SaveBlock1 moved.

    python3 tools/save_convert/expansion_1_17.py OLD.sav NEW.sav

Never modifies OLD.sav and refuses to overwrite NEW.sav. Prints a before/after
report (player, party, boxes, Pokedex, quests, Wally's Ralts) so you can check
nothing was lost. Python 3 standard library only.

What changes (measured with compare_builds.py; see README.md):
- SaveBlock1: dexSeen and dexCaught grow from 114 to 128 bytes each; the
  fields after them (Trainer Hill, Mystery Event script, record mixing gift,
  Lilycove Lady, trainer name records, registered Easy Chat texts, Walda's
  phrase) move 28 bytes later.
- SaveBlock1: 4 unused bytes became dailySeed (lottery, Mirage Island); it gets
  a random value, as on a new day. Mass Outbreak TV shows changed layout and
  are taken off the air.
- Overworld sprite IDs: the 16 dynamic slots OBJ_EVENT_GFX_VAR_0..F moved from
  244-259 to 389-404 (in saved object events and object event templates).
- Everything else is byte-identical: SaveBlock2 (quests), SaveBlock3 (Wally's
  Ralts), the PC, the Pokemon format, and every item, move, species, flag, var,
  map and trainer ID.
"""

import argparse
import os
import struct
import sys

SECTOR_SIZE = 0x1000
SECTOR_DATA_SIZE = 3968
SB3_CHUNK_SIZE = 116
SECTOR_SIGNATURE = 0x08012025
SECTORS_PER_SLOT = 14
NUM_SLOTS = 2
FOOTER = struct.Struct('<HHII')  # id, checksum, signature, counter
FOOTER_OFFSET = SECTOR_DATA_SIZE + SB3_CHUNK_SIZE

ID_SAVEBLOCK2 = 0
ID_SAVEBLOCK1 = range(1, 5)
ID_STORAGE = range(5, 14)

SB2_SIZE = 3908
STORAGE_SIZE = 34144
OLD_SB1_SIZE = 15536
NEW_SB1_SIZE = 15564

# SaveBlock1 (offsets from both builds' DWARF; old -> new)
DEX_SEEN = 13720
OLD_DEX_CAUGHT, NEW_DEX_CAUGHT = 13834, 13848
OLD_DEX_BYTES, NEW_DEX_BYTES = 114, 128
OLD_TAIL, NEW_TAIL = 13948, 13976          # trainerHillTimes .. end of the block
DAILY_SEED = 2500                          # was unused_9C2[2..5]
OBJECT_EVENTS, OBJECT_EVENT_SIZE, OBJECT_EVENT_COUNT, OBJECT_EVENT_GFX = 2608, 36, 16, 4
TEMPLATES, TEMPLATE_SIZE, TEMPLATE_COUNT, TEMPLATE_GFX = 3184, 24, 64, 1
TV_SHOWS, TV_SHOW_SIZE, TV_SHOW_COUNT = 10188, 36, 25
TVSHOW_MASS_OUTBREAK = 41
OLD_GFX_VARS, NEW_GFX_VARS, GFX_VAR_COUNT = 244, 389, 16

# Report fields (same offsets in both versions)
PARTY_COUNT, PARTY, MON_SIZE = 564, 568, 100
BOXES, BOX_MON_SIZE, BOX_MON_COUNT = 4, 80, 14 * 30
PLAYER_NAME, QUESTS, QUESTS_SIZE = 0, 3884, 24
WALLY_RALTS, WALLY_RALTS_SIZE = 16, 12
SB3_SIZE = 28

SUBSTRUCT0_POS = [0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3, 1, 1, 2, 3, 2, 3]


class SaveError(Exception):
    pass


def chunk_size(total, n):
    return max(0, min(total - n * SECTOR_DATA_SIZE, SECTOR_DATA_SIZE))


def sector_sizes(sb1_size):
    sizes = {ID_SAVEBLOCK2: SB2_SIZE}
    for n, sid in enumerate(ID_SAVEBLOCK1):
        sizes[sid] = chunk_size(sb1_size, n)
    for n, sid in enumerate(ID_STORAGE):
        sizes[sid] = chunk_size(STORAGE_SIZE, n)
    return sizes


def checksum(data):
    total = sum(struct.unpack(f'<{len(data) // 4}I', data[:len(data) // 4 * 4])) & 0xFFFFFFFF
    return ((total >> 16) + total) & 0xFFFF


def read_slot(flash, slot, sizes):
    """Sector id -> (physical index, sector bytes), or None if the slot isn't a complete valid save."""
    sectors = {}
    counters = set()
    for i in range(SECTORS_PER_SLOT):
        phys = slot * SECTORS_PER_SLOT + i
        sector = flash[phys * SECTOR_SIZE:(phys + 1) * SECTOR_SIZE]
        sid, csum, sig, counter = FOOTER.unpack_from(sector, FOOTER_OFFSET)
        if sig != SECTOR_SIGNATURE or sid not in sizes or checksum(sector[:sizes[sid]]) != csum:
            return None
        sectors[sid] = (phys, sector)
        counters.add(counter)
    if len(sectors) != SECTORS_PER_SLOT or len(counters) != 1:
        return None
    return sectors


def join(sectors, ids, total):
    return b''.join(sectors[sid][1][:chunk_size(total, n)] for n, sid in enumerate(ids))


def convert_sb1(old):
    new = bytearray(NEW_SB1_SIZE)
    new[:DEX_SEEN] = old[:DEX_SEEN]
    new[DEX_SEEN:DEX_SEEN + OLD_DEX_BYTES] = old[DEX_SEEN:DEX_SEEN + OLD_DEX_BYTES]
    new[NEW_DEX_CAUGHT:NEW_DEX_CAUGHT + OLD_DEX_BYTES] = old[OLD_DEX_CAUGHT:OLD_DEX_CAUGHT + OLD_DEX_BYTES]
    new[NEW_TAIL:] = old[OLD_TAIL:]
    assert len(new) == NEW_SB1_SIZE

    struct.pack_into('<I', new, DAILY_SEED, int.from_bytes(os.urandom(4), 'little'))

    remapped = 0
    for base, size, count, gfx in ((OBJECT_EVENTS, OBJECT_EVENT_SIZE, OBJECT_EVENT_COUNT, OBJECT_EVENT_GFX),
                                   (TEMPLATES, TEMPLATE_SIZE, TEMPLATE_COUNT, TEMPLATE_GFX)):
        for i in range(count):
            at = base + i * size + gfx
            (gfx_id,) = struct.unpack_from('<H', new, at)
            if OLD_GFX_VARS <= gfx_id < OLD_GFX_VARS + GFX_VAR_COUNT:
                struct.pack_into('<H', new, at, gfx_id - OLD_GFX_VARS + NEW_GFX_VARS)
                remapped += 1

    outbreaks = 0
    for i in range(TV_SHOW_COUNT):
        at = TV_SHOWS + i * TV_SHOW_SIZE
        if new[at] == TVSHOW_MASS_OUTBREAK:
            new[at:at + TV_SHOW_SIZE] = bytes(TV_SHOW_SIZE)
            outbreaks += 1
    return bytes(new), remapped, outbreaks


def write_slot(flash, sectors, sb1):
    sizes = sector_sizes(NEW_SB1_SIZE)
    for sid, (phys, sector) in sectors.items():
        sector = bytearray(sector)
        if sid in ID_SAVEBLOCK1:
            n = sid - ID_SAVEBLOCK1[0]
            data = sb1[n * SECTOR_DATA_SIZE:n * SECTOR_DATA_SIZE + sizes[sid]]
            sector[:SECTOR_DATA_SIZE] = data + bytes(SECTOR_DATA_SIZE - len(data))
        _, _, sig, counter = FOOTER.unpack_from(sector, FOOTER_OFFSET)
        FOOTER.pack_into(sector, FOOTER_OFFSET, sid, checksum(bytes(sector[:sizes[sid]])), sig, counter)
        flash[phys * SECTOR_SIZE:(phys + 1) * SECTOR_SIZE] = sector


def box_species(mon):
    """Species of an encrypted BoxPokemon, or 0 if the slot is empty."""
    personality, ot_id = struct.unpack_from('<II', mon, 0)
    if personality == 0 and ot_id == 0 and not any(mon[32:80]):
        return 0
    key = personality ^ ot_id
    pos = SUBSTRUCT0_POS[personality % 24]
    (word,) = struct.unpack_from('<I', mon, 32 + pos * 12)
    return (word ^ key) & 0x7FF


def summary(sb1, sb2, storage, sb3, dex_caught, dex_bytes):
    party = [box_species(sb1[PARTY + i * MON_SIZE:PARTY + i * MON_SIZE + 80]) for i in range(min(sb1[PARTY_COUNT], 6))]
    boxed = [box_species(storage[BOXES + i * BOX_MON_SIZE:BOXES + (i + 1) * BOX_MON_SIZE]) for i in range(BOX_MON_COUNT)]
    popcount = lambda b: sum(bin(x).count('1') for x in b)
    return {
        'player name bytes': sb2[PLAYER_NAME:PLAYER_NAME + 8].hex(),
        'party species': party,
        'boxed Pokemon': sum(1 for s in boxed if s),
        'boxed species checksum': sum(s * (i + 1) for i, s in enumerate(boxed)),
        'dex seen': popcount(sb1[DEX_SEEN:DEX_SEEN + dex_bytes]),
        'dex caught': popcount(sb1[dex_caught:dex_caught + dex_bytes]),
        'quest data': sb2[QUESTS:QUESTS + QUESTS_SIZE].hex(),
        "Wally's Ralts": sb3[WALLY_RALTS:WALLY_RALTS + WALLY_RALTS_SIZE].hex(),
        'SaveBlock2 bytes': checksum(sb2 + bytes(-len(sb2) % 4)),
        'PC bytes': checksum(storage),
    }


def sb3_of(sectors):
    return b''.join(sectors[sid][1][SECTOR_DATA_SIZE:FOOTER_OFFSET] for sid in range(SECTORS_PER_SLOT))[:SB3_SIZE]


def convert(flash):
    flash = bytearray(flash)
    old_sizes = sector_sizes(OLD_SB1_SIZE)
    reports = []
    for slot in range(NUM_SLOTS):
        sectors = read_slot(flash, slot, old_sizes)
        if sectors is None:
            continue
        last_sb1 = sectors[ID_SAVEBLOCK1[-1]][1]
        if any(last_sb1[old_sizes[ID_SAVEBLOCK1[-1]]:sector_sizes(NEW_SB1_SIZE)[ID_SAVEBLOCK1[-1]]]):
            raise SaveError(f'slot {slot + 1} already has data past the old SaveBlock1 end: '
                            'this save looks converted already (or is from a newer build)')
        counter = FOOTER.unpack_from(sectors[0][1], FOOTER_OFFSET)[3]
        sb1 = join(sectors, ID_SAVEBLOCK1, OLD_SB1_SIZE)
        sb2 = join(sectors, [ID_SAVEBLOCK2], SB2_SIZE)
        storage = join(sectors, ID_STORAGE, STORAGE_SIZE)
        sb3 = sb3_of(sectors)
        new_sb1, remapped, outbreaks = convert_sb1(sb1)
        write_slot(flash, sectors, new_sb1)

        check = read_slot(flash, slot, sector_sizes(NEW_SB1_SIZE))
        if check is None:
            raise SaveError(f'slot {slot + 1}: converted slot failed its own checksum check')
        after = summary(join(check, ID_SAVEBLOCK1, NEW_SB1_SIZE), join(check, [ID_SAVEBLOCK2], SB2_SIZE),
                        join(check, ID_STORAGE, STORAGE_SIZE), sb3_of(check), NEW_DEX_CAUGHT, NEW_DEX_BYTES)
        before = summary(sb1, sb2, storage, sb3, OLD_DEX_CAUGHT, OLD_DEX_BYTES)
        reports.append((slot, counter, before, after, remapped, outbreaks))
    if not reports:
        if any(read_slot(flash, slot, sector_sizes(NEW_SB1_SIZE)) for slot in range(NUM_SLOTS)):
            raise SaveError('already in the expansion 1.17.1 format; nothing to convert')
        raise SaveError('no valid save slot found (not an expansion 1.14.1 Verdant Emerald save?)')
    return bytes(flash), reports


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('old', help='save from a pre-1.17.1 build (not modified)')
    ap.add_argument('new', help='converted save to write (must not exist)')
    args = ap.parse_args()
    if os.path.exists(args.new):
        sys.exit(f'{args.new} exists; not overwriting')
    data = open(args.old, 'rb').read()
    if len(data) < NUM_SLOTS * SECTORS_PER_SLOT * SECTOR_SIZE:
        sys.exit(f'{args.old}: {len(data)} bytes, too small for a flash save')
    try:
        converted, reports = convert(data)
    except SaveError as e:
        sys.exit(f'{args.old}: {e}')

    ok = True
    for slot, counter, before, after, remapped, outbreaks in reports:
        print(f'slot {slot + 1} (save count {counter}): {remapped} sprite IDs remapped, {outbreaks} outbreak TV shows removed')
        for key in before:
            same = before[key] == after[key]
            ok &= same
            print(f"  {'ok  ' if same else 'DIFF'} {key}: {before[key]}" + ('' if same else f' -> {after[key]}'))
    if not ok:
        sys.exit('conversion check failed; nothing written')
    with open(args.new, 'xb') as f:
        f.write(converted)
    print(f'wrote {args.new}')


if __name__ == '__main__':
    main()
