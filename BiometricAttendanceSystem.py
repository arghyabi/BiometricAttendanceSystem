import serial
from datetime import datetime
import time

def comDetect():
    try:
        #com=int(input("Enter the port number:"))
        com ='/dev/ttyACM0'
        ser = serial.Serial(com, 9600)

        print(com +" is available. We are initializing the port...")
        time.sleep(10.5)
        print("Port is working now.")
        # print(ser.name)
        while True:
            try:
                a = ser.readline()
                a = a[:-2:]

                b = ""
                for i in a:
                    b = b + chr(i)
                if (len(b) > 0):
                    #print(b)
                    print(str(datetime.now()).split(".")[0]+" : Data receive from hardware.")
                    data = b.split("*")
                    attendace = data[0].split(",")
                    A_time = data[1].split(",")

                    now = datetime.now().strftime("%I_%M_%S_%p")
                    file_name = "document_"+now+".csv"
                    fp = open(file_name, 'w')
                    fp.write("Fingerprint Attendance System\n")
                    fp.write("National Institute of Technology Kurukshetra\n")
                    fp.write("School of VLSI & Embedded System\n")
                    fp.write("Design By: Arghya Biswas & Ashraf Maniyar\n")

                    fp.write("Generated time: " + now + "\n\n\n")

                    fp.write('Roll,Attendance,Time\n')

                    att = 0
                    abb = 0
                    for j in range(len(attendace)):
                        if (str(attendace[j]) == '1'):
                            att = att + 1
                        else:
                            abb = abb + 1
                        fp.write(str(j + 1) + "," + str(attendace[j]) + "," + str(A_time[j]) + "\n")
                    fp.write("\n\n\nPresent = " + str(att) + "\n")
                    fp.write("Absent = " + str(abb) + "\n")
                    fp.write("Total = " + str(len(attendace)))

                    fp.close()
                    print("File Generated. Open the '"+file_name+"' file.")
            except:
                print("Something error happen")

    except:
        print("Wrong input. Only COM number is allow")
        comDetect()

comDetect()
