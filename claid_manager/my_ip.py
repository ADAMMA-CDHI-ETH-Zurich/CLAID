import socket  
def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

def my_ip(*args):

    print("Trying to query IP Address.")
    try:
        ip_address = get_ip_address()
        print("Found IP address of network facing interface:", ip_address)
    except:
        print("Failed to look up local IP address for host. Try using \"ifconfig\" or \"ipconfig\" instead.")

    return
    hostname=socket.gethostname()   
    
    print("Tring to query IP for host {}".format(hostname))

    try:
        IPAddr=socket.gethostbyname(hostname)
        print("IP address of {} is {}".format(hostname, IPAddr))
    except:
        print("Failed to look up local IP address for host. Try using \"ifconfig\" or \"ipconfig\" instead.")