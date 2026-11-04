import QtQuick 2.12


// PBR 材质与 IBL的界面配置；继承公共模块契约，目前不包含实际 GPU 渲染。
LabModule {
    // 定义模块身份、显示标题和渲染实现的接入目录。
    moduleId: "pbr"
    title: "PBR / IBL"
    subtitle: "材质与环境光"
    entryPath: "pbr_lab/render/"
    // 实验导航使用的标题和说明，选择实验只改变界面状态。
    experiments: [
        {
            "title": "材质球阵列",
            "description": "观察金属度与粗糙度的变化，建立材质参数与外观之间的联系。"
        },
        {
            "title": "模型材质",
            "description": "为单个模型接入材质贴图、直接光照与法线映射。"
        },
        {
            "title": "环境光照",
            "description": "接入 HDR 环境贴图，观察漫反射和镜面 IBL。"
        }
    ]
    // 本模块参数默认值；创建模块和点击重置时复制到当前参数。
    defaults: ({"metallic":0,"roughness":0.35,"exposure":1,"environmentRotation":0})
    // 参数编辑器的声明式规格：键名、标签、范围、步长和显示小数位。
    parameterSpecs: [
        {
            "key": "metallic",
            "label": "金属度",
            "from": 0,
            "to": 1,
            "step": 0.01,
            "decimals": 2
        },
        {
            "key": "roughness",
            "label": "粗糙度",
            "from": 0.05,
            "to": 1,
            "step": 0.01,
            "decimals": 2
        },
        {
            "key": "exposure",
            "label": "曝光",
            "from": 0.1,
            "to": 5,
            "step": 0.1,
            "decimals": 1
        },
        {
            "key": "environmentRotation",
            "label": "环境旋转 / °",
            "from": 0,
            "to": 360,
            "step": 1,
            "decimals": 0
        }
    ]
    // 预留渲染管线阶段及接入说明，这些定义不表示对应 GPU Pass 已执行。
    stages: [
        {
            "title": "几何与材质",
            "description": "读取顶点、法线与材质参数，输出用于光照计算的表面信息。"
        },
        {
            "title": "直接光照",
            "description": "接入 Cook-Torrance BRDF 与直接光源。"
        },
        {
            "title": "IBL",
            "description": "接入 Irradiance、预过滤环境贴图与 BRDF LUT。"
        },
        {
            "title": "色调映射",
            "description": "在 HDR 线性空间计算完成后进行曝光和显示转换。"
        }
    ]
}

