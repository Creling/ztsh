from django.urls import path
from . import consumers

websocket_urlpatterns = [
    path('ws/info/', consumers.infoConsumer),
    path('ws/shell/<ip>', consumers.shellConsumer)
]

