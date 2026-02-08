# Tighter Analysis of Password Guessing Curves with Applications to PINs

This is an implementation of the statistical techniques from an anonymous [submission](papers/PIN_Bounds.pdf) (Tighter Analysis of Password Guessing Curves with Applications to PINs). The repository also implements statistical bounds described in [prior work](papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf) of Blocki and Liu (S&P 2023). 

## Introduction

A central challenge in password security is to characterize an attacker’s guessing curve ($\lambda_{G}$) i.e., what is the probability that the attacker will crack a random user’s password within the first $G$ guesses. Guessing curves have important implication towards the development of robust security policies (determining the complexity of hashing algorithms, imposing password security policies, etc.) and the balance between security gains and usability costs. A key challenge is that the guessing curve depends on the attacker’s guessing strategy and the distribution of user passwords, both of which are unknown to us. This line of work aims to follow Kerckhoff's principle by analyzing the performance of an **optimal** attacker who knows the underlying password distribution.

Let $\mathcal{P}$ denote an arbitrary password distribution over passwords $\lbrace pwd_{1}, pwd_{2}, pwd_{3}, \ldots \rbrace$ and $p_{i}$ denote the probability of sampling $pwd_{i}$. Without loss of generality, we can assume that the passwords are ordered by probability ($p_{1} \geq p_{2} \geq \cdots$). Intuitively, when an **optimal** attacker who knows the distribution tries to crack a random password $s \leftarrow \mathcal{P}$ sampled from the distribution, the attacker will make guesses in the order $pwd_{1}, pwd_{2}, \ldots$ until success. The true guessing curve $\lambda_{G}$ can be calculated by summing the probability of the $G$ most probable passwords in the distribution ($\lambda_{G} = \sum_{i \leq G} p_{i}$). Unfortunately, the password distribution $\mathcal{P}$ is unknown to us, so we need to apply statistical techniques to help estimate the true value.

In our application, we focus on generating tight upper and lower bounds for the guessing curve $\lambda_{G}$ of an optimal attacker. We stress that we make **no a priori assumptions** about the shape of the password distribution. Instead, our statistical framework only requires *iid* samples $S = \lbrace s_{1}, s_{2}, \ldots, s_{N} \rbrace \leftarrow \mathcal{P}^{N}$ from the **unknown** distribution. Formally, given samples $S$, guessing number $G$, and error rate $\delta$, our program can output $L$ and $U$ with the guarantee that $\Pr[L \leq \lambda_{G} \leq U] \geq 1 - \delta$.
 
### Specifying a Password Sample

The program can process password samples in 3 different file formats:

- **plain text**: Each line is one password.
- **password-frequency**: Each line is a password and an integer separated by a `\t` character, representing a **unique** password and the number of times it occurs in the sample (i.e. its frequency). To ensure correctness, make sure that no two lines contain the same passwords.
- **frequency-count**: Each line is two integers $x$ and $y$ separated by whitespace, indicating that there are $y$ unique passwords that all occur $x$ times in the sample.

> Note: A `"freqcount"` sample does not contain information about the actual passwords. Therefore, bounds that incorporate password cracking models cannot be applied to such datasets. Specifically, the `extended_LB` bound described in [prior work](papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf) is not available for such samples.

For example, consider a password sample containing 4 passwords: `{ "123456", "abcdef", "Password123", "abcdef" }`. In **plain text** format, it would be

    123456
    abcdef
    Password123
    abcdef

In **password-frequency** format it would be

    123456  1
    abcdef  2
    Password123  1

In **frequency-count** format it would be

    2 1
    1 2

When interacting with the interface, the user can use strings `"plain"`, `"pwdfreq"`, or `"freqcount"` to indicate which format their password sample is represented with.

### Bounding Techniques

We implement four statistical techniques to generate upper/lower bounds $\lambda_G$ for guessing budgets $G$ of varying magnitudes. The first 3 categories are described in [prior work](papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf) and the last one is from the current [submission](papers/PIN_Bounds.pdf). <!--[this paper](papers/PIN_paper).-->

1. Empirical Distribution (Upper Bound): Use the empirical distribution to upper bound $\lambda_G$. `freq_UB` is the bound that applies this technique.
2. Sample Partition (Lower Bound): Partition the sample $S$ into two parts $D_1$ and $D_2$. Use $D_1$ to build a cracking dictionary (ordered by frequency with which these passwords occur in $D_1$) and then measure the fraction of passwords in $D_2$ that would be cracked within $G$ guesses by an attacker that uses this dictionary. This lower bounds $\lambda_G$ as an attacker will perfect knowledge of the distribution can only **outperform** an attacker with partial knowledge of the distribution (the samples in $D_1$). Intuitively, this bound plateaus as the guessing budget $G$ exceeds the number of distinct passowrds in $D_{1}$, we can obtain an extended lowerbound by using a password generating model to make the remaining guesses. Bounds that apply this technique are `samp_LB`, `extended_LB`, and `binom_LB`.
3. Linear Programming (Upper/Lower Bound): Find a distribution that maximizes (resp. minimizes) the quantity $\lambda_G$ subject to the constraint that the distribution is sufficiently consistent with the underyling samples $S$ to obtain an upper (resp. lower) bound. Good-Turing frequency estimatation is used to generate linear constraints that hold with high probability. The bounds that apply this technique are `LP_LB` and `LP_UB`.
4. Binomial PDF (Upper/Lower Bound): Apply properties of the binomial probability density function to generate upper/lower bounds. Empirical analysis indicates that the binomial PDF upper/lower bounds consistently outperforms the empirical distribution upper bound as well as the sample partition lower bound. Linear programming provides the tightest upper/lower bounds when the guesing number is larger (e.g., when $G > N$ is larger than the number of samples) while the binomial upper/lower bounds tend to outperform linear programming when the guessing number is lower. The bounds that apply this technique are `binom_LB` and `binom_UB`.

Our program provides two interfaces. For those who want a quick summary without delving into too many technical details, the [simple interface](#simple-interface) automatically selects parameters that should work well in most settings; for those who wish to optimize parameters to obtain better results or perform customized experiments, the [advanced tools](#advanced-tools) allow full control over the parameters. Please also take a look at the [examples](examples/) for better guidance. For instructions on building and executing the program, see [this section](#usage).

## Simple Interface

The simple interface is an interactive console that allows users to use simple commands to calculate upper/lower bounds of guessing curves and/or generate entire plots of guessing curves without having to write code. The interface has several functionalities:

- Generate the best upper/lower bounds for specific guessing budgets.
- Generate plots of the upper/lower bounds (best bounds or all available bounds) for the entire guessing curve ($\lambda_{G}$ vs. $G$).
- Maintain multiple password samples during the same session for the user to analyze and compare different samples.

Please see the [usage](usage) section for guidance on building and executing the interactive console.

## Advanced Tools

The program utilizes the data structure `dist_t` to represent password samples and store precomputed information about the samples. 

### Creating a `dist_t` object

- Use the `read_file(dist, filename, filetype)` function to associate a password sample stored in `filename` with the object `dist`. `filetype` should be either `"plain"`, `"pwdfreq"`, or `"freqcount"` depending on the format of the password sample file. The function returns `true` if the sample was successfully read and `false` if the file does not exist or couldn't be read.
- Use the `set_verbose(dist, true/false)` function to turn on/off error messages. Error messages are output to `stderr`, one can also redirect them to another file. By default, it is set to true.
- Use the `write_freqcount(dist, filename)` function to write the password sample `dist` in the "freqcount" format to the file `filename`. This is useful since reading a "freqcount" file is much more efficient than reading a file with actual passwords. The function returns `true` is the data was successfully written to the file and `false` otherwise.

### Partitioning a sample

The program provides two ways of partitioning a sample into two sets, which is necessary for calculating tight lower bounds. The first way is partitioning inside the program, and the second way is partitioning before the running program. For samples in the format "freqcount", only the first method is viable; for samples in formates "plain" or "pwdfreq", both methods are viable but the second method is recommended.

It is recommended that the user keep the size of $D_{2}$ relatively small compared to the total number of samples (the paper uses $d = 25000$ with sample sets of size around $10^{8}$) to get the best bounds. Partitioning is required for bounds `samp_LB`, `extended_LB`, and `binom_LB`.

#### Partitioning within the program
 
- Use the `partition(dist, d)` function to randomly partition the sample set $S$ into $D_{1}$ and $D_{2}$ where $D_{2}$ is of size `d` and $D_{1}$ contains the remaining samples. The function returns `true` if the partition was succcessful and `false` otherwise.
- Use the `partition(dist, fraction)` function to randomly partition the sample set $S$ into $D_{1}$ and $D_{2}$ where $D_{2}$ is of size `d = N * fraction` (`N` is size of sample) and $D_{1}$ contains the remaining samples. The function returns `true` if the partition was succcessful and `false` otherwise.

> If the user decides to use this method for samples in the format "pwdfreq" or "plain", one can retrieve the actual passwords in each partitioned set, use the function `partition(dist, d/fraction, D1_filename, D2_filename)` to write the passwords in each dataset into the files `D1_filename` and `D2_filename`. However, it is recommended that the user use the second method listed below for samples in the two formats since the user might need to use the passwords in $D_{1}$ to train a password cracking model for `extended_LB` beforehand.

#### Partitioning before running the program

Use the `pre_partition(dist, d)` function to specify that the password sample file has been pre-partitioned into sets $D_{1}$ and $D_{2}$ where $D_{2}$ is of size `d` and $D_{1}$ contains the remaining samples. A pre-partitioned

### Attacking a sample

After partitioning the sample, one can train a passowrd guessing model using the set $D_{1}$ to generate guesses for `extended_LB` as the guesses number $G$ exceeds the number of distinct passwords in $D_{1}$. Use `model_attack(dist, file)` to attack the password sample where `file` contains a password (the model's guess) on each line. When generating the additional attack attempts with a password model, the user can choose to omit the password guesses from the model that are present in $D_{1}$ since they would already be guessed during the dictionary attack in `samp_LB`.

### Calculating Bounds

All upper/lower bounds available have a basic version `bound_name(dist, G, err)` where `dist` is the `dist_t` object, `G` is the guessing budget, and `err` is the desired error rate, which automatically selects parameters if necessary. The bounds with extra parameters have overloads where the user can manually set them. The output lower bounds $L$ (resp. upper bounds $U$) hold with probability at least `err`.

- `freq_UB(dist, G, err)`: No adjustable parameters.
- `samp_LB`: Requires partition and the size of partition `d` is an adjustable parameter.
- `extended_LB`: Requires partition and a model attack. The size of the partition `d` and the password model used are adjustable parameters.
- `LP_LB`: The mesh granularity `q`, number of linear constraints in the LP `iprime`, error rates `errs`, and slack terms `xhats` are adjustable parameters. Use `LP_LB(dist, G, q, iprime, errs, xhats)` to set parameters. `q` should be a `double` greater than 1, `iprime` should be an integer, `errs` should be a `vector<double>` of size `iprime + 1` that specifies the error rate of each linear constraint in the LP, and `xhats` should be a `vector<double>` of size `iprime + 1`. The final error rate of the lower bound would be two times the sum of the elements of `errs`. For details about the parameters, please see [the paper](./papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf).
- `LP_UB`: The mesh granularity `q`, number of linear constraints in the LP `iprime`, error rates `errs`, and slack terms `xhats` are adjustable parameters. Use `LP_UB(dist, G, q, iprime, errs, xhats)` to set parameters. `q` should be a `double` greater than 1, `iprime` should be an integer, `errs` should be a `vector<double>` of size `iprime + 1` that specifies the error rate of each linear constraint in the LP, and `xhats` should be a `vector<double>` of size `iprime + 1`. The final error rate of the upper bound would be two times the sum of the elements of `errs`. For details about the parameters, please see [the paper](./papers/Towards_a_Rigorous_Statistical_Analysis_of_Empirical_Password_Datasets.pdf).
- `binom_LB`: Requires partition and the size of the partition `d` is an adjustable parameter.
- `binom_UB(dist, G, err)`: No adjustable parameters.

If successful, the functions return a number between 0 and 1, the upper or lower bound. If an error occured, the functions return `-1`, and an error message will be printed to the console if `verbose` is set to true. For the linear programming bounds, the functions will return `-2` if the linear program is infeasible which indicates that the password sample was not sampled *iid* from the underlying distribution. As shown in [examples](examples/), the linear programs ran on the [linkedin frequency corpus](https://figshare.com/articles/dataset/linkedin_files_zip/7350287) sample set are infeasible as the dataset contains duplicate accounts of the same user.

In addition, use the functions `best_LB(dist, G, err)` and `best_UB(dist, G, err)` to get the tightest bound for a specific $G$ value. The functions uses heuristics to avoid performing time consuming linear programs for certain $G$ values where other bounds are certainly better.

### Plotting the Guessing Curves

Use the function `tikz_plot(data, style, legend, file)` to generate latex code for plotting the entire guessing curve ($\lambda_{G}$ vs. $G$). `data` is of type `vector<vector<pair<int64_t, string>>>`, `style` and `legend` is of type `vector<string>`, the three vectors should be of the same length. Each `vector<pair<int64_t, double>>` is a list of $(\lambda_{G}\,,\, G)$ pairs for a certain bound, and the corresponding elements in `style` and `legend` specifies the style and name of the bound. See [examples](examples/) for better guidance.

> Make sure to include `\usepackage{tikz}` and `\usepackage{pfgplot}` in the latex file.

## Usage

### Building the Program

#### Building the Interactive Console

First, create a build directory and switch to the directory.

    mkdir build && cd build

Then, run cmake to generate the build files.

    cmake ../

The file `CmakeLists.txt` contains default install paths for the gurobi c++ library for different operating systems. One can follow the steps of installing the library and obtaining a free academic license [here](https://support.gurobi.com/hc/en-us/articles/14799677517585-Getting-Started-with-Gurobi-Optimizer). If the user did not install the library in the default path, use the `-D` flag to specify where the library is installed on the local machine.

    cmake -DGUROBI_PATH=<user's gurobi path> ../

Then, build the program and switch back to the root directory of the project.

    make interactive && cd ../

Finally, execute the binary to access the interactive console

    ./interactive

#### Building with User's Code

The user can write their own program to access the functions listed in [advanced tools](advanced-tools) (see [examples](examples/examples.cpp) for guidance). Follow the instructions in [CmakeLists.txt](CmakeLists.txt) to add an executable and a source file. Then, use the same commands to build the program.

    mkdir build && cd build
    cmake ../ (or cmake -DGUROBI_PATH=<user's gurobi path> ../)
    make <user's executable name> && cd ../
    ./<user's executable name>

Make sure to include the necessary files with these include statements.

    #include "distribution.hpp"
    #include "pwdio.hpp"
    #include "bounds.hpp"
    #include "lp_bounds.hpp"
    #include "wrappers.hpp"
    #include "plotting.hpp"

### Other Notes
- Please keep the password file associated with a `dist_t` object available and unchanged as long as the object is still in use. The program might read from the file to calculate bounds.
- For consistency and to ensure compilation success, please use `int64_t` for **every** integer variable in the program.

