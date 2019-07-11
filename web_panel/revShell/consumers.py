from channels.generic.websocket import WebsocketConsumer, JsonWebsocketConsumer
from asgiref.sync import async_to_sync
import time
import redis
import socket
from . import shell
import time

re_base = redis.Redis(host='127.0.0.1', port='6379', db=1)
unix_socket_path = '/root/unix_to_ztsh.sock'


class infoConsumer(JsonWebsocketConsumer):
    def connect(self):
        print("connection start!\n")
        self.accept()

    def disconnect(self, close_code):        
        print("connection ends!\n")

    def receive_json(self, content=None):
        while True:
            res = []
            keys = re_base.keys()
            for key in keys:
                res.append([key.decode(), re_base.get(key).decode()])
            self.send_json(res)
            time.sleep(5)


class shellConsumer(WebsocketConsumer):
    def connect(self):
        self.accept()
        self.target_ip = self.scope['url_route']['kwargs']['ip']
        self.socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        try:
            self.socket.connect(unix_socket_path)
        except Exception as e:
            print(e)
        self.socket.send(self.target_ip.encode())

        # init Shell
        self.shell = shell.Shell(self, self.socket)

    def disconnect(self, close_code):
        self.socket.shutdown(socket.SHUT_RDWR)
        self.socket.close()
        print("connection ends!\n")

    def receive(self, text_data=None, bytes_data=None):
        self.shell.hander(text_data)

