    // 显示注册账号的模态框
    function show_create_modal(){
        $("input[name='createusername']").val("");
        $("input[name='createpassword']").val("");
        $("input[name='createemail']").val("");
        document.getElementById("pcreateusername").style.color = 'black';
        document.getElementById("pcreatepassword").style.color = 'black';
        document.getElementById("pcreateemail").style.color = 'black';
        $('#myModal_create').modal('show');
    }

    // 注册按钮
    $("#create").click(function(){
        var user = $("input[name='createusername']").val();
        var code = $("input[name='createpassword']").val();
        var email = $("input[name='createemail']").val();
        // 账号未填写
        if (user==''){
            document.getElementById("pcreateusername").style.color = 'red';
            $("input[name='createusername']").focus();
            return ;
        }
        // 账号名合法
        var uPattern = /^[a-zA-Z0-9_]{4,16}$/;
        if (!uPattern.test(user)){
            document.getElementById("pcreateusername").innerHTML = '请输入合法的用户名';
            document.getElementById("pcreateusername").style.color = 'red';
            $("input[name='createusername']").val("").focus();
            return ;
        }
        // 邮箱未填写
        if (email==''){
            document.getElementById("pcreateemail").style.color = 'red';
            $("input[name='createemail']").focus();
            return ;
        }
        // 邮箱合法
        var ePattern = /^([A-Za-z0-9_\-\.])+\@([A-Za-z0-9_\-\.])+\.([A-Za-z]{2,4})$/;
        if (!ePattern.test(email)){
            document.getElementById("pcreateemail").innerHTML = '请输入合法的邮箱';
            document.getElementById("pcreateemail").style.color = 'red';
            $("input[name='createemail']").val("").focus();
            return ;
        }
        // 密码未填写
        if (code==''){
            document.getElementById("pcreatepassword").style.color = 'red';
            $("input[name='createpassword']").focus();
            return ;
        }
        // ajax请求注册
        $.ajax({
            url:"/createuser",
            type:'POST',
            data:{'user': user, 'email': email,'code': code},
            success: function(arg){
                ret = eval(arg);
                if(ret.status){
                    swal({  
                        type: 'success',  
                        title: '注册成功！',  
                        confirmButtonText: '确定',  
                        confirmButtonColor: '#4cd964'  
                    }).then(function(){
                        // 注册成功后刷新页面
                        window.location.reload();
                    });  
                }else{
                    swal({  
                        type: 'error',  
                        title: '用户名已存在！',  
                        confirmButtonText: '确定', 
                        confirmButtonColor: '#4cd964'  
                    })
                }
            }
        });
    })

