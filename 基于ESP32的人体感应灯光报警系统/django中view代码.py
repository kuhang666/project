from keshe.models import note
from keshe.serializers import noteSerializer
from django.http import HttpResponse
from django.http import Http404
from keshe import models
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import serializers

# 127.0.0.1:8000/test1/{}json
class noteAPI(APIView):
    # 成功
    def get(self,request):
        # 获取所有书籍
        get = note.objects.all()
        # 通过序列化器的转换（模型转换为JSON）
        serializer = noteSerializer(get,many=True)
        # 返回响应
        return Response(serializer.data)

    def post(self,request):
        # 验证参数（序列化器的校验）
        serializer = noteSerializer(data=request.data)
        if serializer.is_valid():
            # 数据入库
            serializer.save()
            return Response(serializer.data)
        else:
            # 返回响应
            return HttpResponse(serializer.errors)

  
 # 127.0.0.1:8000/test2/id/{}json
class  noteAPI2(APIView):
    def get(self, request,id):
        # data = request.GET.get(id=id) 
        get_test= note.objects.get(id=id)
        serializer = noteSerializer(get_test,many=False)
        return Response(serializer.data)

    def put(self, request,id):
        data_put= note.objects.get(id=id)
        serializer = noteSerializer(data=request.data,instance=data_put,many=False)
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data)
    # 成功 request.GET.get('id')先获取了url的id=5，获取数字5
    # request.data是获取 { }json格式

    def delete(self, request,id):

        data_delete =note.objects.get(id=id)
        data_delete.delete()
        return Response("")
