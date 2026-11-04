import QtQuick 2.12


// 延迟渲染与多光源的界面配置；继承公共模块契约，目前不包含实际 GPU 渲染。
LabModule {
    // 定义模块身份、显示标题和渲染实现的接入目录。
    moduleId: "deferred"
    title: "Deferred"
    subtitle: "多光源与 G-Buffer"
    entryPath: "deferred_lab/render/"
    // 实验导航使用的标题和说明，选择实验只改变界面状态。
    experiments: [
        {
            "title": "多光源场景",
            "description": "用同一个场景和相机比较前向与延迟渲染的工作量。"
        },
        {
            "title": "G-Buffer 查看",
            "description": "查看位置、法线、基础色、材质与深度附件。"
        },
        {
            "title": "光源影响范围",
            "description": "可视化点光源的影响范围，接入光体积与剔除。"
        }
    ]
    // 本模块参数默认值；创建模块和点击重置时复制到当前参数。
    defaults: ({"lightCount":32,"lightRadius":5,"objectCount":25,"lightSpeed":1})
    // 参数编辑器的声明式规格：键名、标签、范围、步长和显示小数位。
    parameterSpecs: [
        {
            "key": "lightCount",
            "label": "点光源数量",
            "from": 1,
            "to": 256,
            "step": 1,
            "decimals": 0
        },
        {
            "key": "lightRadius",
            "label": "光源半径",
            "from": 0.5,
            "to": 20,
            "step": 0.1,
            "decimals": 1
        },
        {
            "key": "objectCount",
            "label": "物体数量",
            "from": 1,
            "to": 100,
            "step": 1,
            "decimals": 0
        },
        {
            "key": "lightSpeed",
            "label": "光源运动速度",
            "from": 0,
            "to": 3,
            "step": 0.1,
            "decimals": 1
        }
    ]
    // 预留渲染管线阶段及接入说明，这些定义不表示对应 GPU Pass 已执行。
    stages: [
        {
            "title": "几何阶段",
            "description": "绘制不透明几何体，将表面属性写入 G-Buffer。"
        },
        {
            "title": "G-Buffer",
            "description": "分配并检查多渲染目标的格式、尺寸与完整性。"
        },
        {
            "title": "光照阶段",
            "description": "从 G-Buffer 读取属性，执行多光源光照计算。"
        },
        {
            "title": "最终合成",
            "description": "合成天空盒、前向透明物体与最终显示输出。"
        }
    ]
}

