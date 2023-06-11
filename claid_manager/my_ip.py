import socket  

def my_ip(*args):


    hostname=socket.gethostname()   
    
    print("Tring to query IP for host {}".format(hostname))

    try:
        IPAddr=socket.gethostbyname(hostname)
        print("IP address of {} is {}".format(hostname, IPAddr))
    except:
        print("Failed to look up local IP address for host. Try using \"ifconfig\" or \"ipconfig\" instead.")