import threading


class Shell:
    def __init__(self, websocket, unixsocket):
        self.unixsocket = unixsocket
        self.websocket = websocket

    def hander(self, message):
        threading.Thread(target=self.django_to_ztsh, args=(message,)).start()
        threading.Thread(target=self.ztsh_to_django).start()

    def django_to_ztsh(self, message):  # message is type string
        try:
            self.unixsocket.sendall(message.encode())
        except Exception as e:
            print(e)
            exit()

    def ztsh_to_django(self):
        while True:
            res = self.unixsocket.recv(4096)
            if not res:
                break
            res = res.decode()
            self.websocket.send(text_data=res)
