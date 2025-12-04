---
name: commit-style
description: Git commit message style guide. Reference when creating commits.
---

# Commit Style

**First commit of session:** Run `git log --oneline -5` to review recent examples before committing.

**Title:**
- Sentence case, ends with period
- Imperative mood ("Add X." not "Added X." or "Adds X.")
- Describe the change at a high level

**Body (optional):**
- Use bullet points for multiple related changes
- Include rationale when relevant (especially balance changes)
- Keep it scannable for future git log searches

**Attribution:**
- No auto-generated footers (Co-Authored-By, emoji, etc.)
- Ported code: include source as markdown link in title or body

**Scope:**
- One logical change per commit
- Multiple files okay if they're part of the same change
