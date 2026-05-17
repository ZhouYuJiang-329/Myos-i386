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