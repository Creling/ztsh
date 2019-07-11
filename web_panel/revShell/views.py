from django.shortcuts import render_to_response
from django.contrib.auth.decorators import login_required

# Create your views here.
htmltitle = '反向Shell管理平台'

@login_required(login_url='/login')
def index(request):
    username = request.session.get('username')
    return render_to_response('revshell.html', {'title': htmltitle, 'username': username})

@login_required(login_url='/login')
def shell(request, ip): 
    return render_to_response('shell.html')
