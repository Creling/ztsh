import socket

def webservice():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("0.0.0.0", 8889))
    sock.listen(100)
    print('Websocket server start, wait for connect')
    while True:
        connection, address = sock.accept()
        print(address)

webservice()
