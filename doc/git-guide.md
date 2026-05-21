下面是一份 **Git 操作与作用** 的完整总结，涵盖从基础到进阶的常用命令，以及 Git 在版本控制中的核心价值。

---

## 一、Git 是什么（作用总览）

**Git** 是一个**分布式版本控制系统**，主要作用是：

1. **记录文件变更历史**（谁、什么时间、改了什么）
2. **支持多人协作开发**（分支、合并）
3. **可回溯任意历史版本**
4. **备份与恢复**（本地 + 远程仓库）
5. **代码审查与质量管控**（通过 PR / MR）

> 核心思想：**每次提交都是一份完整的快照**，而不是差异文件。

---

## 二、Git 的三大区域与文件状态

### 1. 三大区域
- **工作区（Working Directory）**：你电脑上实际看到的文件
- **暂存区（Staging Area / Index）**：临时存放修改的地方
- **仓库（Repository）**：最终保存所有历史版本的地方（本地仓库 + 远程仓库）

### 2. 文件四种状态
- **未跟踪（Untracked）**：新文件，从未被 Git 管理
- **已修改（Modified）**：已跟踪文件被修改，但未暂存
- **已暂存（Staged）**：修改已加入暂存区
- **已提交（Committed）**：数据已安全保存在本地仓库

---

## 三、Git 常用操作（按场景分类）

### 1. 初始化与配置
| 命令 | 作用 |
|------|------|
| `git init` | 初始化当前目录为 Git 仓库 |
| `git clone <url>` | 克隆远程仓库到本地 |
| `git config --global user.name "name"` | 设置用户名 |
| `git config --global user.email "email"` | 设置邮箱 |
| `git config --list` | 查看当前配置 |

### 2. 基本操作（本地）
| 命令 | 作用 |
|------|------|
| `git status` | 查看工作区和暂存区状态 |
| `git add <file>` | 将文件添加到暂存区 |
| `git add .` | 添加所有变更文件 |
| `git commit -m "message"` | 提交暂存区内容到仓库 |
| `git commit -am "message"` | 跳过暂存区直接提交（仅已跟踪文件） |
| `git log` | 查看提交历史 |
| `git log --oneline --graph` | 简洁图形化日志 |
| `git diff` | 查看工作区与暂存区差异 |
| `git diff --staged` | 查看暂存区与上次提交差异 |

### 3. 撤销与回退
| 命令 | 作用 |
|------|------|
| `git restore <file>` | 撤销工作区修改（回到上次提交状态） |
| `git restore --staged <file>` | 将文件从暂存区撤回到工作区 |
| `git reset HEAD <file>` | 同上（旧版语法） |
| `git reset --soft HEAD~1` | 撤销一次提交，修改留在暂存区 |
| `git reset --mixed HEAD~1` | 撤销一次提交，修改留在工作区（默认） |
| `git reset --hard HEAD~1` | 彻底删除上一次提交（危险） |
| `git revert HEAD` | 生成一个新的反向提交（安全撤销） |

### 4. 分支管理
| 命令 | 作用 |
|------|------|
| `git branch` | 查看本地分支（当前分支带 *） |
| `git branch -a` | 查看所有分支（含远程） |
| `git branch <name>` | 创建分支 |
| `git checkout <name>` | 切换分支 |
| `git switch <name>` | 切换分支（新语法） |
| `git checkout -b <name>` | 创建并切换分支 |
| `git merge <branch>` | 将指定分支合并到当前分支 |
| `git branch -d <name>` | 删除本地分支 |
| `git push origin --delete <name>` | 删除远程分支 |

### 5. 远程仓库
| 命令 | 作用 |
|------|------|
| `git remote -v` | 查看远程仓库地址 |
| `git remote add origin <url>` | 添加远程仓库 |
| `git push origin <branch>` | 推送分支到远程 |
| `git push -u origin <branch>` | 推送并建立跟踪关系 |
| `git pull origin <branch>` | 拉取并合并（fetch + merge） |
| `git fetch origin` | 仅拉取远程更新（不合并） |
| `git clone <url>` | 克隆远程仓库 |

### 6. 合并与变基
| 命令 | 作用 |
|------|------|
| `git merge <branch>` | 合并分支（产生一个合并提交） |
| `git rebase <branch>` | 将当前分支的提交“嫁接”到目标分支（线性历史） |
| `git rebase --continue` | 解决冲突后继续变基 |
| `git rebase --abort` | 放弃变基 |

> ⚠️ **黄金法则**：不要对公共分支（如 main）执行 rebase

### 7. 暂存与清理
| 命令 | 作用 |
|------|------|
| `git stash` | 暂时保存工作区修改 |
| `git stash list` | 查看 stash 列表 |
| `git stash pop` | 恢复最近 stash 并删除记录 |
| `git stash apply` | 恢复最近 stash 保留记录 |
| `git stash drop` | 删除某个 stash |
| `git clean -fd` | 删除未跟踪的文件和目录 |

### 8. 标签（版本发布）
| 命令 | 作用 |
|------|------|
| `git tag` | 列出所有标签 |
| `git tag v1.0.0` | 创建轻量标签 |
| `git tag -a v1.0.0 -m "message"` | 创建附注标签 |
| `git push origin v1.0.0` | 推送单个标签 |
| `git push origin --tags` | 推送所有标签 |

### 9. 日志与搜索
| 命令 | 作用 |
|------|------|
| `git log --oneline --graph --all` | 图形化全部分支日志 |
| `git blame <file>` | 查看文件每行最后一次修改的提交 |
| `git grep "text"` | 在代码历史中搜索文本 |
| `git show <commit>` | 查看某次提交的详细信息 |

---

## 四、典型工作流示例

### 场景1：个人开发（单分支）
```bash
git init
git add .
git commit -m "first commit"
git remote add origin https://...
git push -u origin main
```

### 场景2：功能分支协作
```bash
git checkout -b feature-login
# 修改代码
git add .
git commit -m "add login"
git push origin feature-login

# 切回主分支合并
git checkout main
git pull origin main
git merge feature-login
git push origin main
```

### 场景3：紧急修复（hotfix）
```bash
git checkout -b hotfix-1.0 main
# 修改 bug
git commit -am "fix critical bug"
git checkout main
git merge hotfix-1.0
git push origin main
```

---

## 五、常见冲突解决思路

1. **冲突产生**：两个分支修改了同一文件的同一区域
2. **查看冲突文件**：`git status`
3. **手动编辑文件**：删除 `<<<<<<<`、`=======`、`>>>>>>>` 标记，保留正确内容
4. **标记已解决**：`git add <file>`
5. **继续合并/变基**：`git merge --continue` 或 `git rebase --continue`

---

## 六、Git 的核心优势（总结作用）

| 作用 | 说明 |
|------|------|
| 历史记录 | 所有修改可追溯、可审计 |
| 分支能力 | 低成本切换上下文，支持并行开发 |
| 分布式 | 每个开发者拥有完整仓库，不依赖中心服务器 |
| 安全 | SHA-1 哈希保证内容完整性 |
| 高性能 | 大部分操作在本地完成，速度极快 |
| 生态强大 | GitHub / GitLab / Gitee 等平台支撑 |

---

## 七、学习建议

- 初学者先掌握：`add`、`commit`、`push`、`pull`、`branch`、`merge`
- 再逐步学习：`rebase`、`stash`、`reset` vs `revert`、`cherry-pick`
- 避免恐惧：所有操作几乎都能撤销（除了 `--hard` 且未提交的情况）

如果你需要一份 **PDF 版** 或 **常用命令速查表（一页纸）**，我也可以帮你整理。

-----------------------git 分支------------------------

## Git 分支：完整指南

---

## 一、分支是什么？（概念理解）

**分支** 就是一条独立的**开发线**，允许你在不干扰主线（或其他分支）的情况下进行修改。

### 形象比喻
- **主线 (main/master)** = 主公路
- **分支 (branch)** = 从主公路分出去的岔路
- 你可以在岔路上修路、测试、建造，不影响主公路的交通
- 建好后可以把岔路**合并**回主公路

### 技术本质
分支本质上只是一个**指针**，指向某个提交对象。创建分支就是创建一个新的指针，几乎不消耗任何额外资源（几十字节）。

```
提交历史：
  A --- B --- C  ← main
         \
          D --- E  ← feature-login
```

---

## 二、分支的核心作用

### 1. **隔离开发**
- 不同功能在不同分支上开发，互不干扰
- Bug 修复、新功能、实验性代码各自独立

### 2. **并行协作**
- 多人可以同时在不同分支上工作
- 不会互相覆盖代码

### 3. **风险控制**
- 主线始终保持稳定可发布状态
- 不稳定的代码只存在分支中

### 4. **快速切换**
- 几秒钟内切换不同上下文（hotfix → feature → main）
- 无需复制整个项目

### 5. **版本管理**
- 每个功能对应一个分支
- 每个发布版本对应一个分支/tag

### 6. **代码审查**
- 通过 Pull Request / Merge Request 进行分支审查
- 团队代码质量保证

---

## 三、分支的常见类型（团队协作标准）

| 分支类型 | 命名示例 | 作用 | 生命周期 |
|---------|---------|------|---------|
| **主分支** | `main` / `master` | 稳定可发布版本 | 永久 |
| **开发分支** | `develop` | 集成最新功能 | 永久 |
| **功能分支** | `feature/login` | 开发新功能 | 临时 |
| **修复分支** | `hotfix/1.0.1` | 紧急修复线上 Bug | 临时 |
| **发布分支** | `release/1.0.0` | 准备发布版本 | 临时 |

### 实际案例：Git Flow 工作流
```
main      :  A --- B --- C --- D --- E
            /         /         /
develop   : A --- B --- C --- D --- E
          /     /     /     /     /
feature   :       F --- G
hotfix    :            H
```

---

## 四、分支的基本操作（从入门到熟练）

### 1. 查看分支
```bash
git branch              # 查看本地分支（* 表示当前分支）
git branch -r           # 查看远程分支
git branch -a           # 查看所有分支（本地+远程）
git branch -v           # 查看每个分支最后一次提交
```

### 2. 创建分支
```bash
git branch feature-login          # 创建分支（停留在当前分支）
git checkout -b feature-login     # 创建并切换到新分支
git switch -c feature-login       # 新语法，同上
```

### 3. 切换分支
```bash
git checkout main                 # 切换到 main 分支
git switch main                   # 新语法，推荐
```

### 4. 合并分支
```bash
# 场景：把 feature-login 合并到 main
git checkout main                 # 先切换到目标分支
git merge feature-login           # 合并源分支
```

#### 合并结果有两种情况：
- **Fast-forward（快进）**：main 没有新提交 → 直接移动指针
- **三方合并**：两个分支都有新提交 → 创建新的合并提交

### 5. 删除分支
```bash
git branch -d feature-login       # 删除本地分支（已合并）
git branch -D feature-login       # 强制删除（未合并也要删）
git push origin --delete feature-login  # 删除远程分支
```

### 6. 解决冲突
当合并时出现冲突：
```bash
git merge feature-login
# 显示：CONFLICT in file.txt

# 1. 打开冲突文件，手动解决（删除 <<<<<<< ======= >>>>>>>）
# 2. 标记已解决
git add file.txt

# 3. 完成合并
git merge --continue
# 或
git commit
```

---

## 五、实战场景演练

### 场景 1：开发新功能

```bash
# 1. 从最新 main 创建功能分支
git checkout main
git pull origin main
git checkout -b feature-payment

# 2. 在分支上开发
# 修改代码...
git add .
git commit -m "add payment gateway"

# 继续开发...
git commit -m "add refund feature"

# 3. 推送远程（备份+协作）
git push origin feature-payment

# 4. 完成后合并回 main
git checkout main
git pull origin main           # 确保 main 最新
git merge feature-payment      # 合并
git push origin main

# 5. 删除本地和远程分支
git branch -d feature-payment
git push origin --delete feature-payment
```

### 场景 2：紧急修复线上 Bug

```bash
# 1. 从 main 创建 hotfix 分支
git checkout main
git checkout -b hotfix-login-error

# 2. 修复 Bug
# 修改代码...
git commit -am "fix login validation error"

# 3. 立即合并到 main
git checkout main
git merge hotfix-login-error
git push origin main

# 4. 同时也要合并到 develop（如果有）
git checkout develop
git merge hotfix-login-error

# 5. 删除分支
git branch -d hotfix-login-error
```

### 场景 3：开发中途需要切到其他分支

```bash
# 当前在 feature-a，但线上有紧急 Bug
git stash                     # 暂存未提交的修改
git checkout main
git checkout -b hotfix-bug

# 修复 Bug 并合并...

# 回到原来的工作
git checkout feature-a
git stash pop                 # 恢复之前的工作
```

---

## 六、分支管理最佳实践

### ✅ 推荐做法

1. **保持 main 分支始终可部署**
   - 只有经过测试的代码才能合并到 main

2. **分支命名规范**
   ```
   feature/user-login    # 功能分支
   bugfix/payment-error  # Bug 修复
   hotfix/critical-crash # 紧急修复
   release/v1.2.0       # 发布分支
   chore/update-readme   # 杂项任务
   ```

3. **频繁提交 + 合并**
   - 每天至少推送一次分支
   - 功能完成后立即合并

4. **合并前先拉取最新代码**
   ```bash
   git checkout main
   git pull origin main
   git checkout feature-xxx
   git merge main          # 或 git rebase main
   ```

5. **删除已合并的分支**
   - 保持仓库干净
   - `git branch --merged` 查看已合并分支

### ❌ 避免做法

1. **长时间不合并** → 冲突越来越多
2. **在 main 上直接修改** → 破坏稳定性
3. **强制推送公共分支** (`git push --force`) → 覆盖他人代码
4. **分支名称混乱** → `test`, `new`, `aaa` 没人看懂

---

## 七、常见问题解答

### Q1：分支太多怎么办？
```bash
# 查看哪些分支已合并到 main
git branch --merged main

# 批量删除已合并分支（除了 main）
git branch --merged main | grep -v "main" | xargs git branch -d
```

### Q2：误删了分支怎么办？
```bash
# 1. 找到删除前的提交哈希
git reflog

# 2. 恢复分支
git checkout -b recovered-branch <commit-hash>
```

### Q3：push 时提示"远程有新提交"怎么办？
```bash
git pull origin main          # 拉取远程更新
# 解决冲突（如有）
git push origin feature-xxx
```

### Q4：怎么知道当前在哪个分支？
```bash
git branch                    # 看到 * 开头的是当前分支
# 或
git status                    # 第一行会显示
```

---

## 八、可视化理解（推荐工具）

### 命令行查看分支图
```bash
git log --oneline --graph --all --decorate
```

### 图形化工具
- **VS Code**：内置 Git 图形界面
- **GitKraken**：专业 Git GUI
- **SourceTree**：免费可视化工具
- **GitHub Desktop**：最简单易用

---

## 九、快速记忆卡

| 操作 | 命令 |
|-----|------|
| 查看分支 | `git branch` |
| 创建分支 | `git branch <name>` |
| 创建并切换 | `git checkout -b <name>` |
| 切换分支 | `git checkout <name>` |
| 合并分支 | `git merge <name>` |
| 删除分支 | `git branch -d <name>` |
| 查看分支图 | `git log --graph --oneline` |

---

## 核心总结

**分支的意义**：让你能在**不影响主线**的前提下**并行开发**，然后用**合并**将成果整合回来。

**使用口诀**：
- 新功能 → 建分支
- 修 Bug → 建分支
- 改完测试 → 合并主
- 合并完事 → 删分支

**记住：分支是最廉价、最强大的隔离机制，大胆用、频繁用！**


--------------------------git 撤销--------------------------------

## Git 撤销与回退：完整指南

---

## 一、核心概念理解

### 为什么要撤销/回退？
- **代码写错了** → 回到正确版本
- **提交错了** → 撤销这次提交
- **合并错了** → 取消合并
- **误删文件** → 恢复文件

### 三个关键区域（回顾）
```
工作区 → 暂存区 → 本地仓库 → 远程仓库
(你看到的样子) (准备提交) (已提交历史) (团队共享)
```

### 四个时间点
```
现在的工作区 ← 暂存区 ← 上次提交(HEAD) ← 更早的提交
```

---

## 二、撤销操作的四种类型

| 操作类型 | 影响范围 | 是否可逆 | 安全等级 |
|---------|---------|---------|---------|
| **撤销工作区修改** | 未暂存的修改 | ⚠️ 部分可逆 | 中 |
| **撤销暂存** | 暂存区 → 工作区 | ✅ 安全 | 高 |
| **撤销提交（保留修改）** | 提交记录 | ✅ 安全 | 高 |
| **彻底回退（丢弃修改）** | 提交历史+修改 | ❌ 危险 | 低 |

---

## 三、场景化操作指南

### 🔵 场景 1：工作区改乱了，想回到干净状态

**情况**：修改了文件但还没 `git add`

```bash
# 撤销单个文件
git restore file.txt

# 撤销所有文件
git restore .

# 或者旧语法
git checkout -- file.txt
```

**意义**：放弃未暂存的修改，回到**上次提交**的状态

---

### 🟢 场景 2：add 错了，想从暂存区撤回来

**情况**：不小心 `git add` 了不该加的文件

```bash
# 撤销单个文件
git restore --staged file.txt

# 撤销所有暂存文件
git restore --staged .

# 旧语法（仍然可用）
git reset HEAD file.txt
git reset HEAD .
```

**效果**：文件从暂存区回到工作区，**内容不变**，只是取消暂存

**意义**：重新整理哪些文件要提交

---

### 🟡 场景 3：提交错了，想撤销这次提交（保留代码）

**情况**：刚 `git commit` 但发现漏了文件或写错提交信息

```bash
# 撤销最近一次提交，修改回到暂存区
git reset --soft HEAD~1

# 撤销最近一次提交，修改回到工作区（默认）
git reset --mixed HEAD~1
# 或简写
git reset HEAD~1

# 撤销最近3次提交
git reset --soft HEAD~3
```

**效果对比**：
- `--soft`：提交没了，代码还在暂存区（`git status` 看到绿色）
- `--mixed`：提交没了，代码在工作区（看到红色，未暂存）

**意义**：可以重新组织提交（比如拆成多个提交，或合并成一个）

---

### 🔴 场景 4：提交错了，想彻底删除（不保留修改）

**情况**：提交的代码完全不要了，想回到上一个版本

```bash
# 彻底删除最近一次提交（代码也丢失）
git reset --hard HEAD~1

# 彻底回到某个历史版本
git reset --hard <commit-hash>
```

**⚠️ 危险！** 未推送到远程的提交会被永久删除

**意义**：完全抛弃错误的提交

---

### 🟠 场景 5：已经 push 到远程，想撤销

**情况**：错误代码已经推送到 GitHub/GitLab

```bash
# ❌ 绝对不要用 reset --hard 然后 force push（会害了队友）

# ✅ 安全做法：用 revert 生成反向提交
git revert HEAD                 # 撤销最近一次提交
git revert <commit-hash>        # 撤销指定提交

# 如果有多个提交要撤销（从旧到新）
git revert <old-hash>..<new-hash>

# 推送撤销结果
git push origin main
```

**`revert` 的工作原理**：
```
原来的提交：A → B → C
执行 git revert B
结果：A → B → C → B' (B' 是撤销 B 的修改)
```

**意义**：保留完整历史，安全撤销，适合团队协作

---

### 🟣 场景 6：想回到某个历史版本查看/测试

**情况**：想看三天前的代码长什么样

```bash
# 1. 找到提交哈希
git log --oneline

# 2. 切换到那个版本（临时查看）
git checkout <commit-hash>

# 3. 看完后回到最新版本
git checkout main

# 如果要在那个版本上继续开发
git checkout -b new-branch <commit-hash>
```

**意义**：回到过去查看、测试，不影响当前工作

---

## 四、详细对比表

### reset 三种模式对比

| 模式 | HEAD 移动 | 暂存区 | 工作区 | 场景 |
|-----|----------|--------|--------|------|
| `--soft` | ✅ | 保留 | 保留 | 重新提交（修改提交信息/合并提交） |
| `--mixed` (默认) | ✅ | 清空 | 保留 | 撤销提交，重新 add 文件 |
| `--hard` | ✅ | 清空 | 清空 | 彻底丢弃，回到干净状态 |

### 示例演示
假设有一个文件 `test.txt`：
```bash
# 初始状态
echo "v1" > test.txt
git add . && git commit -m "v1"

# 修改为 v2
echo "v2" > test.txt
git add .
git commit -m "v2"

# 现在执行 git reset --soft HEAD~1
# 结果：提交记录回到 v1，但 test.txt 内容是 v2 且在暂存区

# 执行 git reset --mixed HEAD~1
# 结果：提交记录回到 v1，test.txt 内容是 v2 但未暂存（红色）

# 执行 git reset --hard HEAD~1
# 结果：提交记录回到 v1，test.txt 内容是 v1（彻底丢失 v2）
```

---

## 五、revert vs reset 终极对比

| 对比维度 | revert | reset |
|---------|--------|-------|
| **是否修改历史** | ❌ 不修改（新增提交） | ✅ 修改（删除提交） |
| **是否可推送到远程** | ✅ 安全推送 | ❌ 需要 force push |
| **团队协作** | ✅ 推荐 | ❌ 不推荐（除非个人分支） |
| **原理** | 创建反向提交 | 移动分支指针 |
| **撤销已 push 的提交** | ✅ 可以 | ❌ 危险 |

### 选择建议
- **已 push 到远程** → 用 `revert`
- **只在本地，想保留修改** → 用 `reset --soft/mixed`
- **只在本地，想彻底删除** → 用 `reset --hard`
- **个人开发分支** → 随便用
- **团队公共分支** → 只用 `revert`

---

## 六、高级撤销技巧

### 1. 修改最近一次提交（推荐）
```bash
# 场景：提交后发现漏了文件或写错信息
git add forgotten-file.txt
git commit --amend -m "新的提交信息"

# 效果：合并到上一次提交，不产生新提交
```

### 2. 交互式 rebase（修改多个提交）
```bash
# 修改最近3次提交
git rebase -i HEAD~3

# 在编辑器中：
# pick → 保留
# reword → 修改提交信息
# edit → 修改提交内容
# squash → 合并到上一个提交
# drop → 删除提交
```

### 3. 恢复误删的文件
```bash
# 从最新提交恢复
git restore deleted-file.txt

# 从历史版本恢复
git checkout <commit-hash> -- deleted-file.txt
```

### 4. 撤销 merge
```bash
# 方法1：reset 回到 merge 前（如果未 push）
git reset --hard HEAD~1

# 方法2：revert merge（如果已 push）
git revert -m 1 <merge-commit-hash>
# -m 1 表示保留第一个父分支（通常是 main）的代码
```

### 5. 查看操作历史（救命稻草）
```bash
# 所有 HEAD 移动记录（包括 reset、checkout 等）
git reflog

# 输出示例：
# abc123 HEAD@{0}: reset: moving to HEAD~1
# def456 HEAD@{1}: commit: 错误提交

# 使用 reflog 恢复误删的提交
git reset --hard HEAD@{1}
```

---

## 七、实战场景演练

### 场景 A：提交后想撤销，但要保留代码
```bash
# 1. 撤销提交（代码回到工作区）
git reset HEAD~1

# 2. 重新 add 需要的文件
git add file1.txt

# 3. 重新提交
git commit -m "正确的提交"
```

### 场景 B：最后一次提交错了，直接修改
```bash
# 1. 修改文件
vim file.txt

# 2. 添加到暂存区
git add file.txt

# 3. 合并到上一次提交
git commit --amend --no-edit   # 不修改提交信息
# 或
git commit --amend -m "新的信息"
```

### 场景 C：已经 push 的错误，安全撤销
```bash
# 1. 本地生成反向提交
git revert HEAD

# 2. 正常推送（不用 force）
git push origin main

# 结果：远程历史变成 A-B-C-D，D 是撤销 C 的提交
```

### 场景 D：彻底回到三天前的版本（丢弃所有后续修改）
```bash
# 1. 查看三天的提交
git log --oneline --since="3 days ago"

# 2. 假设找到哈希为 a1b2c3d
git reset --hard a1b2c3d

# 3. 如果已 push，需要强制推送（⚠️ 慎用）
git push --force-with-lease origin main
# --force-with-lease 比 --force 更安全，会检查远程是否有人推送新代码
```

---

## 八、常见错误与解决

| 错误操作 | 后果 | 解救方法 |
|---------|------|---------|
| `git reset --hard` 丢了代码 | 代码消失 | `git reflog` 找回 |
| `git push --force` 覆盖队友代码 | 队友工作丢失 | 让队友 `git reflog` 恢复 |
| 提交了密码/敏感文件 | 历史中有密码 | `git filter-branch` 或 BFG（高级） |
| merge 冲突乱掉了 | 合并混乱 | `git merge --abort` |

---

## 九、决策流程图

```
发现需要撤销
    │
    ▼
是否已 push 到远程？
    │
    ├─ 是 ──→ 用 git revert（安全）
    │
    └─ 否 ──→ 是否要保留修改？
                │
                ├─ 保留修改 ──→ git reset --soft/mixed
                │
                └─ 不保留 ──→ git reset --hard
```

---

## 十、快速记忆卡

| 你想做什么 | 命令 |
|-----------|------|
| 撤销工作区修改 | `git restore <file>` |
| 取消暂存 | `git restore --staged <file>` |
| 撤销提交（保留修改） | `git reset --soft HEAD~1` |
| 撤销提交（不保留修改） | `git reset --hard HEAD~1` |
| 已 push，安全撤销 | `git revert HEAD` |
| 修改最后一次提交 | `git commit --amend` |
| 救回误删的提交 | `git reflog` + `git reset --hard` |
| 放弃合并冲突 | `git merge --abort` |

---

## 核心总结

**撤销 ≠ 删除**，撤销是 Git 给你的**后悔药**。

**三大原则**：
1. **本地随意用 reset**（个人分支）
2. **远程只用 revert**（公共分支）
3. **reflog 是最后的救命稻草**

**记住口诀**：
- 工作区乱了 → `restore`
- add 错了 → `restore --staged`
- 提交错了（本地） → `reset`
- 推送错了（远程） → `revert`
- 想改最后一次 → `amend`


Git commit message 通常使用 **Conventional Commits** 规范，格式如下：


## 常用 Type

| Type | 含义 | 使用场景 |
|------|------|---------|
| `feat` | 新功能 | 新增功能、特性 |
| `fix` | 修复 | 修复 bug |
| `docs` | 文档 | 修改文档、注释 |
| `style` | 格式 | 代码格式调整（空格、缩进等，不影响功能）|
| `refactor` | 重构 | 代码重构，既不修复 bug 也不添加功能 |
| `perf` | 性能 | 性能优化 |
| `test` | 测试 | 添加或修改测试 |
| `chore` | 杂项 | 构建过程、辅助工具变动 |
| `build` | 构建 | 影响构建系统或外部依赖 |
| `ci` | CI | 持续集成配置修改 |

## 示例

```bash
# 修复 bug
git commit -m "fix: 修复 read_hard_disk 函数缺少 ret 指令"

# 新增功能
git commit -m "feat: 添加 setup.asm 引导加载"

# 修改文档
git commit -m "docs: 添加栈与函数调用文档"

# 重构
git commit -m "refactor: 优化 Makefile 使用通配符规则"

# 修改构建
git commit -m "build: 修改硬盘镜像生成方式，支持多扇区写入"
```

