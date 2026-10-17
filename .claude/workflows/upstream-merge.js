export const meta = {
  name: 'upstream-merge',
  description: 'Staged merge of pokeemerald-expansion releases into an upgrade branch: per step merge, migrate, build, test, then an independent audit for lost customizations and ID collisions',
  whenToUse: 'Upgrading Verdant Emerald to a newer pokeemerald-expansion release. Procedure and rules: .claude/skills/upstream-merge/SKILL.md',
  phases: [
    { title: 'Merge', detail: 'one step per upstream tag: merge, migrate, build, test, push' },
    { title: 'Audit', detail: 'independent check per step, repair if needed' },
  ],
}

// args: {
//   worktree: '/home/user/pve-upgrade',
//   branch: 'upgrade-1.18.2',
//   logDir: '<scratchpad>/upgrade',
//   baseline: 'make check result on master before the upgrade (totals and failing tests)',
//   steps: [{ tag: '1.18.2', prev: '1.17.1', changelogs: ['1.18.0', '1.18.1', '1.18.2'], migrations: '1.18.x' }],
// }

const WT = args.worktree
const BRANCH = args.branch
const LOG = args.logDir

const RULES = `
GROUND RULES:
- Read .claude/skills/upstream-merge/SKILL.md in ${WT} first and follow it: setup, resume check, resolution rules, customization checklist.
- Work ONLY inside the git worktree ${WT} (branch ${BRANCH}). Never touch another checkout, never commit to or push master. Never force-push, rebase or amend pushed commits.
- Upstream release tags are local refs refs/upstream/<version>. Do not fetch.
- Build: 'make -j4 > LOGFILE 2>&1', then read the log. Never pipe make output. Tests: 'make check -j4 > LOGFILE 2>&1'. Logs go in ${LOG}/.
- Baseline test results (master before the upgrade): ${args.baseline}
- Commits follow .claude/skills/commit-style/SKILL.md. Merge commits are titled 'Merge pokeemerald-expansion <version>.'.
- Push with 'git push -u origin ${BRANCH}' after each completed step; on network errors only, retry up to 4 times with 2s/4s/8s/16s backoff.
`

const STEP_SCHEMA = {
  type: 'object',
  properties: {
    status: { type: 'string', enum: ['ok', 'blocked'] },
    resumed: { type: 'string' },
    preMergeHead: { type: 'string' },
    head: { type: 'string' },
    conflicts: { type: 'array', items: { type: 'object', properties: {
      file: { type: 'string' }, cause: { type: 'string' }, resolution: { type: 'string' } }, required: ['file', 'cause', 'resolution'] } },
    migrationsRun: { type: 'array', items: { type: 'string' } },
    ourCodeBreakages: { type: 'array', items: { type: 'object', properties: {
      file: { type: 'string' }, cause: { type: 'string' }, fix: { type: 'string' } }, required: ['file', 'cause', 'fix'] } },
    buildPassed: { type: 'boolean' },
    testSummary: { type: 'string' },
    newTestFailures: { type: 'array', items: { type: 'object', properties: {
      test: { type: 'string' }, triage: { type: 'string' } }, required: ['test', 'triage'] } },
    configDecisions: { type: 'array', items: { type: 'object', properties: {
      option: { type: 'string' }, value: { type: 'string' }, note: { type: 'string' } }, required: ['option', 'value', 'note'] } },
    needsUserDecision: { type: 'array', items: { type: 'string' } },
    blockers: { type: 'array', items: { type: 'string' } },
    pushed: { type: 'boolean' },
  },
  required: ['status', 'preMergeHead', 'head', 'conflicts', 'buildPassed', 'testSummary', 'blockers'],
}

const VERIFY_SCHEMA = {
  type: 'object',
  properties: {
    ok: { type: 'boolean' },
    lostCustomizations: { type: 'array', items: { type: 'object', properties: {
      file: { type: 'string' }, what: { type: 'string' }, evidence: { type: 'string' } }, required: ['file', 'what', 'evidence'] } },
    idCollisions: { type: 'array', items: { type: 'object', properties: {
      id: { type: 'string' }, ourUse: { type: 'string' }, upstreamUse: { type: 'string' } }, required: ['id', 'ourUse', 'upstreamUse'] } },
    leftovers: { type: 'array', items: { type: 'string' } },
    buildPassed: { type: 'boolean' },
    notes: { type: 'array', items: { type: 'string' } },
  },
  required: ['ok', 'lostCustomizations', 'idCollisions', 'buildPassed'],
}

function mergePrompt(step, n) {
  return `Step ${n}/${args.steps.length} of a staged pokeemerald-expansion upgrade for the Verdant Emerald ROM hack: merge upstream ${step.tag} into branch ${BRANCH} (currently based on ${step.prev}).
${RULES}
PROCEDURE:
1. cd ${WT}; run the skill's Setup git config commands; confirm branch ${BRANCH}.
2. RESUME CHECK (skill, "Resuming an interrupted merge"): look for a merge commit whose second parent is refs/upstream/${step.tag} on ${BRANCH} or wip/${step.tag}-interrupted, and for a merge in progress. If the merge already exists, do not redo it: fast-forward to the WIP branch if needed, set resumed to what you found, and continue from the first step not yet committed. Otherwise record preMergeHead = 'git rev-parse HEAD' and continue.
3. Read the changelogs for this range from the tag: ${step.changelogs.map(v => `'git show refs/upstream/${step.tag}:docs/changelogs/${v.replace(/\.\d+$/, '.x')}/${v}.md'`).join(', ')}. Focus on breaking changes, migration instructions, config renames, required 'make clean', save-format changes, whitespace/format sweeps.
4. 'git -c merge.renameLimit=20000 merge --no-ff --no-commit refs/upstream/${step.tag}' (add -Xignore-space-change if the changelogs mention a whitespace/format sweep). Resolve every conflict per the skill's resolution rules and customization checklist. List new gameplay-changing config defaults in needsUserDecision.
5. Run the upstream migration scripts for ${step.migrations} (read each one's docs first, review its diff, separate commit; note irrelevant ones).
6. Build (make clean first if the changelogs require it). Fix every error, adapting OUR code to upstream's API. Logical commits.
7. Run 'make check'. For each failure not in the baseline: fix it if the merge caused it; if it comes from an intentional customization, leave it and say why.
8. Push.
If a design decision can't be made safely (an upstream feature replacing ours, an ID collision with no obvious safe move, a save layout clash), resolve conservatively when safe and list it in needsUserDecision. If it blocks a building tree, return status 'blocked', leaving the worktree coherent (committed, or 'git merge --abort' after 'git rerere').
Record EVERY conflicted file in 'conflicts' (what upstream changed vs what we had changed, and the resolution) and every compile/link break in our code in 'ourCodeBreakages'.`
}

function verifyPrompt(step, res) {
  return `You are an independent, skeptical auditor of a merge of pokeemerald-expansion ${step.tag} into branch ${BRANCH} in ${WT}. Assume something was lost until shown otherwise. Do NOT modify tracked files or commit.
Read .claude/skills/upstream-merge/SKILL.md ("Our customizations") first.
The merging agent reported: ${JSON.stringify(res)}
Pre-merge head: ${res && res.preMergeHead}. Previous upstream base: refs/upstream/${step.prev}. New upstream base: refs/upstream/${step.tag}.
CHECKS:
1. Lost customizations: compare 'git diff refs/upstream/${step.prev} ${res && res.preMergeHead}' with 'git diff refs/upstream/${step.tag} HEAD' file by file and hunk by hunk. Every change must survive, be matched by upstream, or have moved somewhere named.
2. ID collisions: every flag, var, script opcode, item, species/form, trainer, map, string ID or save field we added or repurposed must be unused by refs/upstream/${step.tag}.
3. Leftovers: conflict markers, generated files or build outputs committed, *.backup files, broken includes.
4. Build: 'make -j4 > ${LOG}/${step.tag}-verify-build.log 2>&1' in ${WT} must succeed. Then 'TESTS="Verdant:" make check -j4 > ${LOG}/${step.tag}-verify-tests.log 2>&1' must pass.
Return ok=true only if nothing was lost, there are no collisions, and the build and Verdant tests pass.`
}

function fixPrompt(step, verdict, res) {
  return `Repair the ${step.tag} merge on branch ${BRANCH} in ${WT}. An independent audit found problems: ${JSON.stringify(verdict)}
Original merge report: ${JSON.stringify(res)}
${RULES}
Restore every lost customization (re-derive it from 'git diff refs/upstream/${step.prev} ${res && res.preMergeHead}'), resolve every ID collision (move OUR side to a value upstream does not use, update every reference, note old and new values), and clean leftovers. Rebuild, run 'make check', compare to the baseline, commit in logical commits (e.g. 'Restore <feature> lost in expansion ${step.tag} merge.'), push. Return the merge report shape, with preMergeHead = ${res && res.preMergeHead} and conflicts listing what you repaired.`
}

const results = []
let stoppedAt = null
let n = 0
for (const step of args.steps) {
  n++
  const res = await agent(mergePrompt(step, n), { label: `merge:${step.tag}`, phase: 'Merge', schema: STEP_SCHEMA })
  if (!res || res.status !== 'ok') {
    results.push({ step: step.tag, merge: res, verify: null })
    stoppedAt = step.tag
    log(`Stopped at ${step.tag}: merge step did not complete`)
    break
  }
  let verdict = await agent(verifyPrompt(step, res), { label: `audit:${step.tag}`, phase: 'Audit', schema: VERIFY_SCHEMA })
  let fix = null
  if (!verdict || !verdict.ok) {
    log(`${step.tag}: audit found issues, running repair`)
    fix = await agent(fixPrompt(step, verdict, res), { label: `fix:${step.tag}`, phase: 'Audit', schema: STEP_SCHEMA })
    verdict = await agent(verifyPrompt(step, res), { label: `reaudit:${step.tag}`, phase: 'Audit', schema: VERIFY_SCHEMA })
  }
  results.push({ step: step.tag, merge: res, fix, verify: verdict })
  if (!verdict || !verdict.ok) {
    stoppedAt = step.tag
    log(`Stopped at ${step.tag}: audit still failing after repair`)
    break
  }
  log(`${step.tag} merged, built and verified`)
}

return { stoppedAt, results }
