### 操控方式
按w前进, 鼠标左/右键按住拖动前进视角
### 效果节点(倒序)
1. PointLight(no shadow)
![img](README_img/PointLight.png)
2. SkeletalMesh
![img](README_img/SkeletalMesh2.png)
2. Bloom
![img](README_img/Bloom.png)
3. Shadow Map
![img](README_img/ShadowMap2.png)

### 工程结构
1. Actor-ActorComponent
   Entity-Component-System, 逻辑在Actor中, 数据在Component中
2. Scene
   所有Actor目前存放在Scene中
3. RHI
   dynamic render hardware interface
4. multithreading
   分为主线程(game逻辑)和Render线程
5. tripple buffering
   在FrameResourceManager中有三个buffer可以存放三组渲染数据, 因此可以接受主线程提交三组命令

# 参考目录
1. rastertek.com
   http://www.rastertek.com/
2. 龙书demo
3. dx12官方demo
4. learnopengl
   https://learnopengl-cn.readthedocs.io/zh/latest/

# 引擎持续更新计划
1. 新功能
   1. 材质系统设计
      1. 多材质扩展性
         1. ue4材质节点图生成的hlsl代码拷过来，复制代码到本地shader代码的自定义区，即可出ue4导过来的新材质
         2. 引擎需要知道blendmode，shading model，material domain
            1. 因为不同材质不仅影响到shader代码的不同，还与创建RR时管线设置，renderer中渲染顺序有关
            2. blendmode与UE4的靠齐
            3. shading model，material domain暂时不考虑
      2. 加diffuse,specular,ambient贴图
      3. 加法线贴图
      4. 加cube map
      5. 加天空盒
      6. 加入点光源的点阴影映射
      7. 加抗锯齿
      8. 加描边
      9. 加环境映射
      10. 加AO-参考龙书
      11. 把龙书的特性demo都加上来
2.  新特性
    1.  让灯光可以选择旋转
    2.  导出模型文件里面加上包围盒大小，用于计算平行光的位置
    3.  写一个真正的third person 的相机tick，现在的只是绕定点环绕，不会跟character朝向保持一致
3.  性能优化
    1.  做性能profile
    2.  给点光源做一个最大光照范围，超出范围的物体不需要渲染点光源光照
    3.  在game线程加一个机制：如果物体在视锥体之外，就不提交绘制它的drawcall
    4.  相同的pso不用反复提交
    5.  push_back改成emplace_back
    6.  只需要3x4矩阵
    7.  可以处理成多线程的东西写成多线程
4.  设计优化
    1.  FrameResource里面存的都是渲染要用的东西，包括Geometry和RenderResource，Sampler和texture，后两者可以放到rr里面吗？有必要吗？
        1.  暂时认为:静态的资源，因为不会产生gpu读和cpu写的竞争所以放在哪都可以
5.  代码管理
    1.  fmt第三方库用一下
    2.  vs的format
6.  疑问
    1.  场景中有任意多个光源，被光照的模型怎么写shader，光源数量可以动态传入吗？还是hardcode一个很大的值做长度
    2.  为什么说DX12对多线程有很好的支持认识还不够深入
        1.  与DX11的差异如何导致他在多线程上优于DX11？
    3.  文件引用关系怎么管理
        1.  下图怎么把这两个include去掉(RHI.h里面)![img](README_img/WrongInclude.png)
7.  BUG
    1.  找到人物胸前有小片的原因，修复
    2.  退出报错
        1.  D3D12 ERROR: ID3D12Resource2::<final-release>: CORRUPTION: An ID3D12Resource object (0x0000024FD9DD20C0:'ConstantBuffer') is referenced by GPU operations in-flight on Command Queue (0x0000024FD88D1B90:'Unnamed ID3D12CommandQueue Object').  It is not safe to final-release objects that may have GPU operations pending.  This can result in application instability. [ EXECUTION ERROR #921: OBJECT_DELETED_WHILE_STILL_IN_USE] D3D12: **BREAK** enabled for the previous message, which was: [ ERROR EXECUTION #921: OBJECT_DELETED_WHILE_STILL_IN_USE ] Exception thrown at 0x00007FFE1C254B89 (KernelBase.dll) in D3D12DrawMesh.exe: 0x0000087A (parameters: 0x0000000000000001, 0x0000007FF793A2A0, 0x0000007FF793C070). The thread 0x5170 has exited with code 0 (0x0). Unhandled exception at 0x00007FFE1C254B89 (KernelBase.dll) in D3D12DrawMesh.exe: 0x0000087A (parameters: 0x0000000000000001, 0x0000007FF793A2A0, 0x0000007FF793C070).
    3.  RenderDoc显示有疑问
    4.  多一个相同sampler的问题

renderTaskNum有问题，看一下，研究下会不会影响帧率

<!-- ### 相机操作方式
1. ↑↓←→或WASD控制相机位置
2. 按住鼠标左右键调整视角
3. QE上升与下降 -->
