import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

def main():
    # read from valuecost100.0000000.txt which contains a data point B,Bmin,Bmax per line. plot (B,Bmin) in blue and (B,Bmax) in red
    # the file is in the format:
    # B,Bmin,Bmax
    # read the file

    values = ["5000", "10000","50000"]
    marks = ["o", "s", "^"]
    colors = ["b", "g", "r"]
    with open("optimal-bounds.txt", "r") as f:
        optBounds = f.readlines()
    # split the lines into B,Bmin,Bmax
    optData = [line.split(",") for line in optBounds]
    # convert the data to float
    optData = [[float(x) for x in line] for line in optData]
    # split the data into B,Bmin,Bmax
    # B = [line[0] for line in optData[0:20]]
    # Bmin = [line[1] for line in optData[0:20]]
    # Bmax = [line[2] for line in optData[0:20]]
    # # plot the optimal bounds
    # # plt.plot(B, Bmin, "b-", label="lambda_B Lower Bound")
    # # plt.plot(B, Bmax, "g-", label="lambda_B Upper Bound")
    # plt.plot(B, Bmin, "b-", label="lambda_B Lower Bound")
    # plt.plot(B, Bmax, "g-", label="lambda_B Upper Bound")
    for value in values:
        print(value)
        
        
        
        with open(f"pwd_cracked/large{value}.txt", "r") as f:
            pwdBounds = f.readlines()
        # split the lines into B,Bmin,Bmax
        pwdData = [line.split(",") for line in pwdBounds]
        # convert the data to float
        pwdData = [[float(x) for x in line] for line in pwdData]
        # split the data into B,Bmin,Bmax
        B2 = [line[0] for line in pwdData if line[1] != 0 or line[2] != 0]

        #filter out any bmin/bmax == 0
        Bmin2 = [line[1] for line in pwdData if line[1] != 0 or line[2] != 0]
        Bmax2 = [line[2] for line in pwdData if line[1] != 0 or line[2] != 0]

        
        plotB = [B2[-1],B2[-1]]
        plotBVals = [Bmin2[-1], Bmax2[-1]]

        # plot just these points
        plt.plot(plotB, plotBVals, f"{colors[values.index(value)]}-o", label=f"v/k = {value}")
        # plt.plot(B2, Bmin2, f"{colors[values.index(value)]}{marks[values.index(value)]}", label=f"Rational Lower ({value})")
        # plt.plot(B2, Bmax2, f"{colors[values.index(value)]}{marks[values.index(value)]}", label="Rational Upper Bound")

    # add labels and title
    plt.xlabel("Guessing Number B")
    plt.ylabel("Percentage of Passwords")
    plt.title(f"Best (Large) Value Cost Ratio Upper and Lower Bounds")
    plt.legend()
    # show the plot
    print("foor")
    plt.show()
    print("foo")
    # save the plot
    plt.savefig(f"valuecostLarge.png")
    


    #do the same for another file, but plot bMin bMax as points
   
    # close the plot
    plt.close()
    # return the data
  

    return 

main()
