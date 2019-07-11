from django.shortcuts import render_to_response, redirect
from django.http import JsonResponse
from django.contrib.auth.models import User
from django.contrib.auth import login, authenticate
from django.views.decorators.csrf import csrf_exempt
from django.contrib.auth.decorators import login_required

@csrf_exempt
def loginpage(request):
    return render_to_response("login.html")


@csrf_exempt
def createuser(request):
    ret = {'status': False, 'reason': ''}
    if request.method == 'POST':
        username = request.POST.get('user')
        email = request.POST.get('email')
        password = request.POST.get('code')
        try:
            user = User.objects.create_user(username, email, password)
            user.save()
        except Exception:
            ret['reason'] = 'repeated'
            ret['status'] = False
            return JsonResponse(ret)
        ret['status'] = True
    return JsonResponse(ret)


@csrf_exempt
def userlogin(request):
    ret = {'status': False, 'reason': ''}
    if request.method == 'POST':
        username = request.POST.get('username')
        password = request.POST.get('password')
        user = authenticate(username=username, password=password)
        if user is not None:
            request.session['username'] = username
            if user.is_active:
                login(request, user)
                ret['status'] = True
        else:
            ret['reason'] = 'Wrong'
    return JsonResponse(ret)


htmltitle = '反向Shell管理平台'


def index(request):
    return redirect("home/")



@login_required(login_url='/login')
def home(request):
    username = request.session.get('username')
    return render_to_response("home.html", {'title': htmltitle, 'username': username})

@login_required(login_url='/login')
def realtimelog(request):
    username = request.session.get('username')
    return render_to_response("realtimelog.html", {'title': htmltitle, 'username': username})

@login_required(login_url='/login')
def help(request):
    username = request.session.get('username')
    return render_to_response("help.html", {'title': htmltitle, 'username': username})


