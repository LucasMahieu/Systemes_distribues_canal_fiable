import subprocess
import fileinput
import matplotlib.pyplot as plt


# Creation du fichier de log
logFile = "logFileTimeOut"
subprocess.call("rm " + logFile, shell=True)
subprocess.call("touch " + logFile, shell=True)



# nombre de valeurs pour faire la moyenne
moyenne = 2

#detecter la ligne a changer
detect_line = "#define TIMEOUT_WAIT_ACK"
# Values for the window
from_value = 1000
to_value = 10000
jump = 5000



# Pour les courbes
X = list()
tempY = list()
Y = list()



#time to wait between to tests
TTW1 = 1
#time to wait for the test to finish
TTW2 = 5


for i in range(from_value, to_value+1, jump):

    X.append(i)

    # Modify the size of the window
    for line in fileinput.input("./canal/headers/window.h", inplace=True): 
        if detect_line in line:
            print(detect_line + " " + str(i))
        else:
            print(line[:-1])

    # # Modify the port to use
    # for line in fileinput.input("./canal/headers/structure.h", inplace=True): 
    #     if detect_line2 in line:
    #         print(detect_line2 + " " + str(from_value2 + (cnt2%modulo)))
    #         cnt2 += 1
    #     else:
    #         print(line[0:-1])

    subprocess.call("make clean", shell=True)
    subprocess.call("make", shell=True)
    for j in range(moyenne):
        subprocess.call("echo \" \ntimeout, iteration = " + str(i) + ", " + str(j) + "\" >> " + logFile, shell=True)
        procA = subprocess.Popen("make exeA", shell=True, close_fds=True)
        procB = subprocess.Popen("make exeB 2>> " + logFile, shell=True, close_fds=True)
        subprocess.call("sleep " + str(TTW2), shell=True)
        # subprocess.call(["kill", "-9", "%d" % procA.pid])
        # subprocess.call(["kill", "-9", "%d" % procB.pid])
        subprocess.call("killall myCanal", shell=True)
        subprocess.call("sleep " + str(TTW1), shell=True)


# On detecte la ligne avec la chaine :
detect_line3 = "bit mesur"
# On plot
for line in fileinput.input("./" + logFile, inplace=True): 
    if detect_line3 in line:
        tempY.append(int(line[line.find("=")+2:line.find(".")]))
    print(line[:-1])

print("Valeurs de la courbe : ")
print(X)

cnt=0
tmpSum=0
for i in tempY:
    cnt += 1
    tmpSum += i
    if cnt==moyenne:
        Y.append(tmpSum/moyenne)
        cnt = 0
        tmpSum=0

print(Y)

plt.plot(X, Y)
# plt.title("Danger de la vitesse")
# plt.xlabel('Vitesse')
# plt.ylabel('Temps')
plt.show()



