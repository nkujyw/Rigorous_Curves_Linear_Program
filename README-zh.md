# 针对密码猜测曲线的更紧致分析及其在 PIN 码上的应用

这是来自一篇匿名[投稿](papers/PIN_Bounds.pdf)（针对密码猜测曲线的更紧致分析及其在 PIN 码上的应用）中的统计技术的实现。该仓库还实现了 Blocki 和 Liu (S&P 2023) 在[先前工作](papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf)中描述的统计界限。

## 简介

密码安全中的一个核心挑战是刻画攻击者的猜测曲线（$\lambda_{G}$），即攻击者在前 $G$ 次猜测中破解随机用户密码的概率是多少。猜测曲线对制定稳健的安全策略（确定哈希算法的复杂性、强制执行密码安全策略等）以及在安全增益与可用性成本之间取得平衡具有重要意义。一个关键的挑战是，猜测曲线取决于攻击者的猜测策略和用户密码的分布，而这两者对我们来说都是未知的。这一系列工作旨在遵循柯克霍夫原则（Kerckhoff's principle），通过分析知晓潜在密码分布的**最优**攻击者的表现来进行研究。

令 $\mathcal{P}$ 表示密码集合 $\lbrace pwd_{1}, pwd_{2}, pwd_{3}, \ldots \rbrace$ 上的任意密码分布，$p_{i}$ 表示采样到 $pwd_{i}$ 的概率。不失一般性，我们可以假设密码按概率排序（$p_{1} \geq p_{2} \geq \cdots$）。直观地说，当一个知晓分布的**最优**攻击者试图破解从分布中采样的随机密码 $s \leftarrow \mathcal{P}$ 时，攻击者将按照 $pwd_{1}, pwd_{2}, \ldots$ 的顺序进行猜测，直到成功。真实的猜测曲线 $\lambda_{G}$ 可以通过对分布中 $G$ 个最可能密码的概率求和来计算（$\lambda_{G} = \sum_{i \leq G} p_{i}$）。不幸的是，密码分布 $\mathcal{P}$ 对我们来说是未知的，因此我们需要应用统计技术来帮助估计真实值。

在我们的应用中，我们专注于为最优攻击者的猜测曲线 $\lambda_{G}$ 生成紧致的上限和下限。我们强调，我们对密码分布的形状**不做任何先验假设**。相反，我们的统计框架仅需要来自**未知**分布的 *iid*（独立同分布）样本 $S = \lbrace s_{1}, s_{2}, \ldots, s_{N} \rbrace \leftarrow \mathcal{P}^{N}$。形式上，给定样本 $S$、猜测次数 $G$ 和错误率 $\delta$，我们的程序可以输出 $L$ 和 $U$，并保证 $\Pr[L \leq \lambda_{G} \leq U] \geq 1 - \delta$。

### 指定密码样本

程序可以处理 3 种不同文件格式的密码样本：

- **纯文本 (plain text)**：每一行是一个密码。
- **密码-频率 (password-frequency)**：每一行是一个密码和一个整数，以 `\t` 字符分隔，代表一个**唯一**密码及其在样本中出现的次数（即其频率）。为确保正确性，请确保没有两行包含相同的密码。
- **频率-计数 (frequency-count)**：每一行是两个以空格分隔的整数 $x$ 和 $y$，表示有 $y$ 个唯一密码在样本中都出现了 $x$ 次。

> 注意：`"freqcount"` 样本不包含有关实际密码的信息。因此，结合密码破解模型的界限不能应用于此类数据集。具体而言，[先前工作](papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf)中描述的 `extended_LB` 界限对此类样本不可用。

例如，考虑一个包含 4 个密码的密码样本：`{ "123456", "abcdef", "Password123", "abcdef" }`。在 **纯文本 (plain text)** 格式中，它将是：

    123456
    abcdef
    Password123
    abcdef

在 **密码-频率 (password-frequency)** 格式中，它将是：

    123456  1
    abcdef  2
    Password123  1

在 **频率-计数 (frequency-count)** 格式中，它将是：

    2 1
    1 2

与界面交互时，用户可以使用字符串 `"plain"`、`"pwdfreq"` 或 `"freqcount"` 来指示其密码样本使用的格式。

### 边界技术

我们实现了四种统计技术，以为不同量级的猜测预算 $G$ 生成 $\lambda_G$ 的上限/下限。前 3 类在[先前工作](papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf)中已有描述，最后一类来自当前的[投稿](papers/PIN_Bounds.pdf)。1. 经验分布（上限）：使用经验分布作为 $\lambda_G$ 的上限。`freq_UB` 是应用此技术的界限。
2. 样本划分（下限）：将样本 $S$ 划分为两部分 $D_1$ 和 $D_2$。使用 $D_1$ 构建破解字典（按这些密码在 $D_1$ 中出现的频率排序），然后测量 $D_2$ 中有多少比例的密码会被使用此字典的攻击者在 $G$ 次猜测内破解。这构成了 $\lambda_G$ 的下限，因为完全了解分布的攻击者只能**优于**部分了解分布（$D_1$ 中的样本）的攻击者。直观地说，当猜测预算 $G$ 超过 $D_{1}$ 中不同密码的数量时，此界限会趋于平稳，我们可以通过使用密码生成模型进行剩余的猜测来获得扩展的下限。应用此技术的界限有 `samp_LB`、`extended_LB` 和 `binom_LB`。
3. 线性规划（上限/下限）：找到一个分布，使其在满足与底层样本 $S$ 足够一致的约束条件下，最大化（或最小化）量 $\lambda_G$，从而获得上限（或下限）。使用 Good-Turing 频率估计生成高概率成立的线性约束。应用此技术的界限有 `LP_LB` 和 `LP_UB`。
4. 二项式概率密度函数（上限/下限）：应用二项式概率密度函数的性质生成上限/下限。经验分析表明，二项式 PDF 上限/下限始终优于经验分布上限以及样本划分下限。当猜测次数较大（例如，当 $G > N$ 大于样本数时），线性规划提供最紧致的上限/下限，而当猜测次数较低时，二项式上限/下限往往优于线性规划。应用此技术的界限有 `binom_LB` 和 `binom_UB`。

我们的程序提供两个接口。对于那些想要快速摘要而不深入太多技术细节的人，[简单接口](#simple-interface)会自动选择在大多数设置下应该有效的参数；对于那些希望优化参数以获得更好结果或执行自定义实验的人，[高级工具](#advanced-tools)允许完全控制参数。也请查看[示例](examples/)以获得更好的指导。关于构建和执行程序的说明，请参阅[此部分](#usage)。

## 简单接口

简单接口是一个交互式控制台，允许用户使用简单的命令计算猜测曲线的上限/下限和/或生成整个猜测曲线（$\lambda_{G}$ 对 $G$）的绘图，而无需编写代码。该接口具有多种功能：

- 为特定的猜测预算生成最佳上限/下限。
- 为整个猜测曲线（$\lambda_{G}$ 对 $G$）生成上限/下限（最佳界限或所有可用界限）的绘图。
- 在同一会话期间维护多个密码样本，供用户分析和比较不同的样本。

请参阅[用法](usage)部分以获取关于构建和执行交互式控制台的指导。

## 高级工具

程序利用数据结构 `dist_t` 来表示密码样本并存储有关样本的预计算信息。

### 创建 `dist_t` 对象

- 使用 `read_file(dist, filename, filetype)` 函数将存储在 `filename` 中的密码样本与对象 `dist` 关联。`filetype` 应为 `"plain"`、`"pwdfreq"` 或 `"freqcount"`，具体取决于密码样本文件的格式。如果成功读取样本，函数返回 `true`，如果文件不存在或无法读取，则返回 `false`。
- 使用 `set_verbose(dist, true/false)` 函数开启/关闭错误消息。错误消息输出到 `stderr`，也可以将其重定向到另一个文件。默认情况下，设置为 true。
- 使用 `write_freqcount(dist, filename)` 函数将密码样本 `dist` 以 "freqcount" 格式写入文件 `filename`。这很有用，因为读取 "freqcount" 文件比读取包含实际密码的文件效率高得多。如果数据成功写入文件，函数返回 `true`，否则返回 `false`。

### 划分样本

程序提供两种将样本划分为两个集合的方法，这对于计算紧致的下限是必要的。第一种方法是在程序内部进行划分，第二种方法是在运行程序之前进行划分。对于 "freqcount" 格式的样本，只有第一种方法可行；对于 "plain" 或 "pwdfreq" 格式的样本，两种方法都可行，但建议使用第二种方法。

建议用户保持 $D_{2}$ 的大小相对于样本总数较小（论文中使用 $d = 25000$，样本集大小约为 $10^{8}$），以获得最佳界限。`samp_LB`、`extended_LB` 和 `binom_LB` 界限需要进行划分。

#### 在程序内划分

- 使用 `partition(dist, d)` 函数将样本集 $S$ 随机划分为 $D_{1}$ 和 $D_{2}$，其中 $D_{2}$ 的大小为 `d`，$D_{1}$ 包含剩余样本。如果划分成功，函数返回 `true`，否则返回 `false`。
- 使用 `partition(dist, fraction)` 函数将样本集 $S$ 随机划分为 $D_{1}$ 和 $D_{2}$，其中 $D_{2}$ 的大小为 `d = N * fraction`（`N` 是样本大小），$D_{1}$ 包含剩余样本。如果划分成功，函数返回 `true`，否则返回 `false`。

> 如果用户决定对 "pwdfreq" 或 "plain" 格式的样本使用此方法，可以检索每个划分集合中的实际密码，使用函数 `partition(dist, d/fraction, D1_filename, D2_filename)` 将每个数据集中的密码写入文件 `D1_filename` 和 `D2_filename`。但是，建议用户对这两种格式的样本使用下面列出的第二种方法，因为用户可能需要预先使用 $D_{1}$ 中的密码来为 `extended_LB` 训练密码破解模型。

#### 在运行程序前划分

使用 `pre_partition(dist, d)` 函数指定密码样本文件已预先划分为集合 $D_{1}$ 和 $D_{2}$，其中 $D_{2}$ 的大小为 `d`，$D_{1}$ 包含剩余样本。

### 攻击样本

划分样本后，可以使用集合 $D_{1}$ 训练密码猜测模型，以便当猜测次数 $G$ 超过 $D_{1}$ 中不同密码的数量时，为 `extended_LB` 生成猜测。使用 `model_attack(dist, file)` 攻击密码样本，其中 `file` 每一行包含一个密码（模型的猜测）。当使用密码模型生成额外的攻击尝试时，用户可以选择忽略模型中已存在于 $D_{1}$ 中的密码猜测，因为它们在 `samp_LB` 的字典攻击期间已经被猜测过了。

### 计算界限

所有可用的上限/下限都有一个基本版本 `bound_name(dist, G, err)`，其中 `dist` 是 `dist_t` 对象，`G` 是猜测预算，`err` 是期望的错误率，它会自动选择参数（如果需要）。带有额外参数的界限有重载版本，用户可以手动设置这些参数。输出的下限 $L$（或上限 $U$）至少以概率 `err` 成立。

- `freq_UB(dist, G, err)`: 无可调参数。
- `samp_LB`: 需要划分，划分大小 `d` 是一个可调参数。
- `extended_LB`: 需要划分和模型攻击。划分大小 `d` 和使用的密码模型是可调参数。
- `LP_LB`: 网格粒度 `q`、LP 中的线性约束数量 `iprime`、错误率 `errs` 和松弛项 `xhats` 是可调参数。使用 `LP_LB(dist, G, q, iprime, errs, xhats)` 设置参数。`q` 应为大于 1 的 `double`，`iprime` 应为整数，`errs` 应为大小为 `iprime + 1` 的 `vector<double>`，指定 LP 中每个线性约束的错误率，`xhats` 应为大小为 `iprime + 1` 的 `vector<double>`。下限的最终错误率将是 `errs` 元素之和的两倍。有关参数的详细信息，请参阅[论文](./papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf)。
- `LP_UB`: 网格粒度 `q`、LP 中的线性约束数量 `iprime`、错误率 `errs` 和松弛项 `xhats` 是可调参数。使用 `LP_UB(dist, G, q, iprime, errs, xhats)` 设置参数。`q` 应为大于 1 的 `double`，`iprime` 应为整数，`errs` 应为大小为 `iprime + 1` 的 `vector<double>`，指定 LP 中每个线性约束的错误率，`xhats` 应为大小为 `iprime + 1` 的 `vector<double>`。上限的最终错误率将是 `errs` 元素之和的两倍。有关参数的详细信息，请参阅[论文](./papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf)。
- `binom_LB`: 需要划分，划分大小 `d` 是一个可调参数。
- `binom_UB(dist, G, err)`: 无可调参数。

如果成功，函数返回一个 0 到 1 之间的数字，即上限或下限。如果发生错误，函数返回 `-1`，如果 `verbose` 设置为 true，错误消息将打印到控制台。对于线性规划界限，如果线性规划不可行，函数将返回 `-2`，这表明密码样本不是从底层分布中 *iid* 采样的。如[示例](examples/)所示，在 [linkedin 频率语料库](https://figshare.com/articles/dataset/linkedin_files_zip/7350287)样本集上运行的线性程序不可行，因为该数据集包含同一用户的重复账户。

此外，使用函数 `best_LB(dist, G, err)` 和 `best_UB(dist, G, err)` 获取特定 $G$ 值的最紧致界限。这些函数使用启发式方法，避免对某些肯定有其他更好界限的 $G$ 值执行耗时的线性规划。

### 绘制猜测曲线

使用函数 `tikz_plot(data, style, legend, file)` 生成用于绘制整个猜测曲线（$\lambda_{G}$ 对 $G$）的 latex 代码。`data` 类型为 `vector<vector<pair<int64_t, string>>>`，`style` 和 `legend` 类型为 `vector<string>`，这三个向量的长度应相同。每个 `vector<pair<int64_t, double>>` 是特定界限的 $(\lambda_{G}\,,\, G)$ 对列表，`style` 和 `legend` 中的相应元素指定了该界限的样式和名称。请参阅[示例](examples/)以获得更好的指导。

> 确保在 latex 文件中包含 `\usepackage{tikz}` 和 `\usepackage{pfgplot}`。

## 用法

### 构建程序

#### 构建交互式控制台

首先，创建一个构建目录并切换到该目录。

    mkdir build && cd build

然后，运行 cmake 生成构建文件。

    cmake ../

文件 `CmakeLists.txt` 包含了 gurobi c++ 库在不同操作系统下的默认安装路径。用户可以按照[此处](https://support.gurobi.com/hc/en-us/articles/14799677517585-Getting-Started-with-Gurobi-Optimizer)的步骤安装库并获得免费的学术许可证。如果用户未将库安装在默认路径中，请使用 `-D` 标志指定库在本地机器上的安装位置。

    cmake -DGUROBI_PATH=<user's gurobi path> ../

然后，构建程序并切换回项目的根目录。

    make interactive && cd ../

最后，执行二进制文件以访问交互式控制台。

    ./interactive

#### 使用用户的代码构建

用户可以编写自己的程序来访问[高级工具](advanced-tools)中列出的函数（参见[示例](examples/examples.cpp)以获得指导）。按照 [CmakeLists.txt](CmakeLists.txt) 中的说明添加可执行文件和源文件。然后，使用相同的命令构建程序。

    mkdir build && cd build
    cmake ../ (or cmake -DGUROBI_PATH=<user's gurobi path> ../)
    make <user's executable name> && cd ../
    ./<user's executable name>

确保使用这些 include 语句包含必要的文件。

    #include "distribution.hpp"
    #include "pwdio.hpp"
    #include "bounds.hpp"
    #include "lp_bounds.hpp"
    #include "wrappers.hpp"
    #include "plotting.hpp"

### 其他说明
- 请保持与 `dist_t` 对象关联的密码文件可用且未更改，只要该对象仍在使用中。程序可能会读取该文件以计算界限。
- 为了保持一致性并确保编译成功，请在程序中的**每个**整数变量都使用 `int64_t`。