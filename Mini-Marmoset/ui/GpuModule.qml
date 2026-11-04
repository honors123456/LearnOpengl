import QtQuick 2.12


// GPU 实例与模拟的界面配置；继承公共模块契约，目前不包含实际 GPU 渲染。
LabModule {
    // 定义模块身份、显示标题和渲染实现的接入目录。
    moduleId: "gpu"
    title: "GPU Simulation"
    subtitle: "粒子与大规模实例"
    entryPath: "gpu_lab/render/"
    // 实验导航使用的标题和说明，选择实验只改变界面状态。
    experiments: [
        {
            "title": "粒子模拟",
            "description": "接入粒子发射、生命周期与 GPU 并行更新。"
        },
        {
            "title": "实例化草海",
            "description": "接入实例数据、风场动画与批量绘制。"
        },
        {
            "title": "剔除与 LOD",
            "description": "观察可见性剔除和细节层级对绘制成本的影响。"
        }
    ]
    // 本模块参数默认值；创建模块和点击重置时复制到当前参数。
    defaults: ({"instanceCount":100000,"simulationSpeed":1,"windStrength":1,"seed":42})
    // 参数编辑器的声明式规格：键名、标签、范围、步长和显示小数位。
    parameterSpecs: [
        {
            "key": "instanceCount",
            "label": "实例数量",
            "from": 1000,
            "to": 1000000,
            "step": 1000,
            "decimals": 0
        },
        {
            "key": "simulationSpeed",
            "label": "模拟速度",
            "from": 0,
            "to": 3,
            "step": 0.1,
            "decimals": 1
        },
        {
            "key": "windStrength",
            "label": "风力强度",
            "from": 0,
            "to": 5,
            "step": 0.1,
            "decimals": 1
        },
        {
            "key": "seed",
            "label": "随机种子",
            "from": 0,
            "to": 9999,
            "step": 1,
            "decimals": 0
        }
    ]
    // 预留渲染管线阶段及接入说明，这些定义不表示对应 GPU Pass 已执行。
    stages: [
        {
            "title": "实例数据",
            "description": "定义粒子或草叶的实例布局，创建实例缓冲区或 SSBO。"
        },
        {
            "title": "并行更新",
            "description": "在 Compute Shader 中更新模拟数据并设置内存屏障。"
        },
        {
            "title": "可见性处理",
            "description": "接入视锥剔除、LOD 和可选的间接绘制命令生成。"
        },
        {
            "title": "实例绘制",
            "description": "通过实例化绘制读取 GPU 数据，提交最终画面。"
        }
    ]
}

