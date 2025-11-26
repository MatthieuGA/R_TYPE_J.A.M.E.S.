# CONTRIBUTING.md

Thank you for your interest in contributing to **R_TYPE_J.A.M.E.S.**
This document explains how to work on the project, how branches and commits must be structured, how pull requests are reviewed, and how CI & Git hooks ensure code quality.

---

# 📌 **1. Workflow Overview**

We use a simplified GitHub Flow with protected branches, PR reviews, and automated CI.
All contributions must follow the rules below.

### Main rules

* **Never commit directly to `main`**
* **Every change = an Issue + a Feature Branch + a Pull Request**
* **At least 1 reviewer required**
* **CI must pass before merging**
* **Git hooks must be executed locally**

---

## 📁 2. Branching Strategy

We use the following branches:

### Main Branches

| Branch | Purpose                                                      |
| ------ | ------------------------------------------------------------ |
| `main` | Stable production branch. Protected, no direct pushes.       |
| `dev`  | Integration branch where all features merge before releases. |

### Feature Branches

Every task must be implemented in a **dedicated branch**:

```txt
feature/<issue-number>-short-description
refactor/<issue-number>-short-description
bugfix/<issue-number>-short-description
docs/<issue-number>-short-description
```

**Examples:**

```txt
feature/42-implement-ecs-systems
bugfix/88-fix-snapshot-ordering
docs/15-update-architecture-md
```

---

## 📝 3. Commit Rules

### Commit Format

We use **Gitmoji + English commit message**:

```txt
<gitmoji> [scope] Your message in English
```

**Examples:**

```txt
✨ [Engine] Add MovementSystem
🐛 [Network] Fix UDP packet handler crash
📝 [Docs] Update protocol RFC
♻️ [ECS] Refactor component registration
```

### **Commit Messages Must:**

* be written in **English**
* be short and descriptive
* reference the issue when relevant (e.g., `#42`)

---

## 🔐 4. Protected Branch Rules

### Branch protection:

* `main` is **protected**
* Direct push is **forbidden**
* PR approval required
* CI must be green
* Squash & merge required for feature branches

---

## 🔄 5. Pull Request Process

### Step-by-step:

1. **Create an issue**

   * Describe what you will implement
   * Add labels: area, type, priority
   * Add milestone

2. **Create a new feature branch**

   ```txt
   git checkout -b feature/XX-my-feature
   ```

3. **Push your work**

   ```txt
   git push -u origin feature/XX-my-feature
   ```

4. **Open a Pull Request**

   * Use the PR template
   * Link the PR to the issue (`Closes #XX`)
   * Explain what changed & how to test
   * Add screenshots if relevant
   * Assign at least **1 reviewer**

5. **Pass all CI checks**

   * Formatting (clang-format)
   * Build
   * Tests
   * Static checks

6. **Get review & approval**
   Reviewer checks:

   * code quality
   * design consistency
   * compliance with architecture & RFCs
   * correctness

7. **Merge**

   * Use **Squash & Merge**
   * The resulting commit message must stay clean & readable

---

## 🛠️ 6. Git Hooks & Formatting

This project ships with local Git hooks that contributors **must install**.
They ensure code consistency *before* PR creation.

### Included hooks:

| Hook         | Purpose                                                    |
| ------------ | ---------------------------------------------------------- |
| `pre-commit` | Checks formatting (clang-format), lint, forbidden patterns |
| `commit-msg` | Validates commit format (Gitmoji + English message)        |

### Install the hooks:

```
git config core.hooksPath .githooks
```

Hooks run automatically on every commit.

---

## 🔍 7. CI (GitHub Actions)

Every Pull Request triggers the CI pipeline:

### CI Tasks

* ⬛ **Build** (server, client, engine)
* 🧪 **Unit tests**
* 🧪 **Functional tests**
* 🎨 **Formatting (clang-format)**
* 🔗 **Static checks**
* 🚫 **Reject PRs targeting `main` directly**
* 📦 **Release mirroring** (for tags)

A PR cannot be merged if **any** CI job fails.

---

## 🗂️ 8. Issues Guidelines

### Creating an Issue

Each issue should:

* have a clear title
* follow the issue template
* include reproduction steps (if bug)
* include acceptance criteria
* include labels:

  * **area** (client, server, docs, gameplay…)
  * **type** (feature, bug, refactor, docs, test)
  * **priority** (P0–P3)
* be assigned to a milestone
* be assigned to a contributor (optional)

### Issue Size

Tasks must be **small**:

* ideally **1 day of work**
* avoid oversized issues → split into smaller ones

---

## 🔧 9. Code Style

* C++23 required
* Follow **Google Style (clang-format)**
* No unused includes
* No raw pointers unless strictly required
* ECS components must remain **plain data**
* Systems must contain **logic only**
* No coupling between client & server code
* No circular dependencies

---

## 🧪 10. Testing Requirements

Each feature must include tests when applicable:

* **Unit tests** for engine & server logic
* **Functional tests** for client/server interactions
* **Regression tests** when fixing a bug

Tests must pass before merging.

---

## 📦 11. Releases

Releases are prepared by maintainers and follow:

* Semantic versioning
* Milestone completion
* Updated changelog
* Packaging tests
* CI validation

Example milestones:

* `v0.5.0` — MVP
* `v1.0.0` — Final Release

---

## 🤝 12. Getting Help

If you have questions:

* Open a **Discussion**
* Ask in the team communication channel
* Tag maintainers in an Issue

We are here to help contributors succeed 🚀
