# Filling Issues

Issues are filled for the following reasons:
1. Tracking of your own planning and TODOs
2. Visibility for the rest of the team.
3. Getting a better estimate of time needed for epics or overarching features.

This reduces overheads in communication between teams. It provides the neccessary information in negotiating blocking work and keeps everone updated on progress.
Reason (2) and (3) are compelling enough for me to strongly recommend that your work should be tracked with issues. 
If you're filling an issue related to code you're going to write, please 

## Assigned

Assign an issue to someone when you know the owner / are ready to work on it.

## Labels
See https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/labels on label descriptions as to which label should be used.

## Project
Attach your issues to a project so they are properly sorted and have better visibilty. There are too many issues to refer to in the issues view.

## Milestone
Attach your issues to a milestone so we have a rough gauge of it's deadline alongside it's priority label.

# Code Review for Contributor

## PR Description
Your PR description should describe the behaviour you're trying to add to the code base if it's a `feature request`.
If it's a `bug`, it should mention what bug it is and how it fixes it. 

The rule of thumb is, the PR should help the reviewer. It gives teh reviewer enough context to know what to look out for in your code.

## PR Size
_These lines don't include binary files_

- Extra small <= 10 lines
- Small <= 200 line
- Medium <= 500 lines
- Large <= 1000 lines
- Xtra Large <= 5000 lines
- Who hurt you? > 5000 lines

Smaller PRs are preferred. 
**Please aim for Small PRs**

## Draft
Create your PRs as a draft. Mark it as ready to be reviewed only when it is. If you are making changes, mark it as a draft again until it is ready.

## Linked Issue
Ideally each PR should have a linked issue, if an issue for it does not exist, consider creating one for documentation and providing team wide visibility.

## Other fields in a PR
Your PR should come with appropriate Assigned, Project and Milestone fields. 
Adding these fields would help to properly communicate intention and give proper visibility for your work. 

## Reviewers
The CODEOWNER file at the moment is a bit wonky. If it doesn't work properly, please ensure you have the approval from the following people

- CPP @wn, @remo5000
- PKB @wn, @remo5000, @jeffkwoh
- QP @doittomorrow, @huichun66, @eleenyang
- Dev Tools @doittomorrow, @jeffkwoh
- Testing @eleenyang

## Unit Tests

Source code for SPA written should have accompanying unit tests. I.e. code to run CI and other code written for dev tools don't count.
Unit tests should go under the directory `Team00/Code00/src/unit_testing/`

### Exceptions
If a PR is created and wants to be merged without accompanying tests, an issue should be filed to follow up on those tests.
The following are acceptable reasons for not writing unit tests:
- The code written is needed to unblock other tasks
- We are close to a project submission deadline, and there is an accompanying AUTOTESTER test to handle the added behaviour
- Testing is too hard and not viable. PM @jeffkwoh when this happens.

## Integration Tests

Reviewers should ask for integration tests where appropriate.

## System Tests/E2E Tests/AUTOTESTER Tests

@eleenyang is in charge of systems tests. Please work with her if systems tests are failing in your PR. 
For PRs that makes a feature testable E2E, please check with @eleenyang to make sure an E2E test provides coverage.

## Squash and Merge

We squash and merge here. Please reformat your description as neccessary when squashing,

# Code Review for Reviewers

## Turn around time

Expected max turn around time for code reviews is 1 day. Use your own discretion for `P0` and `P1` issues.

## LGTM without Approval

You should comment LGTM without an approval if the code, well, looks good to you, but you're not in the best position to approve.
When this happens please tag someone else to approve. E.g. "LGTM, @remo5000 for approval".

## Nit

Out of courtesy use `nit` to mean nitpicking. E.g.
```
nit: Move this method higher up in the file as it seems more important
```
You should only use `nit` if you are okay with the contributor not making the requested change. 

# Recommended Practices

## Avoid TODO(user) in code base

Avoid TODOs like TODO(user): if possible. TODOs should be linked to an issue in Github. e.g. 
```
// TODO(https://github.com/nus-cs3203/team24-cp-spa-20s1/issues/29)
// Add lexer tests.
```
This ensures that all TODO work is tracked on Github for visibility. Also others reading your code can refer to the issue for more context.

## Avoid we
Don't use `we` in code documentation. Who is "we"? What's the POV "we" is adopting?
