import QtQuick 2.12


// SSAO 采样与环境光合成的界面配置；继承公共模块契约，目前不包含实际 GPU 渲染。
LabModule {
    // 定义模块身份、显示标题和渲染实现的接入目录。
    moduleId: "ssao"
    title: "SSAO"
    subtitle: "屏幕空间环境遮蔽"
    entryPath: "ssao_lab/render/"
    // 实验导航使用的标题和说明，选择实验只改变界面状态。
    experiments: [
        {
            "title": "接触与墙角",
            "description": "观察贴地物体、墙角与凹槽处的局部环境遮蔽。"
        },
        {
            "title": "采样与噪声",
            "description": "比较采样数、采样半径和噪声旋转对 AO 的影响。"
        },
        {
            "title": "降噪与合成",
            "description": "查看原始 AO、降噪结果与最终环境光合成。"
        }
    ]
    // 本模块参数默认值；创建模块和点击重置时复制到当前参数。
    defaults: ({"sampleCount":32,"radius":0.5,"bias":0.025,"intensity":1})
    // 参数编辑器的声明式规格：键名、标签、范围、步长和显示小数位。
    parameterSpecs: [
        {
            "key": "sampleCount",
            "label": "采样数量",
            "from": 8,
            "to": 64,
            "step": 1,
            "decimals": 0
        },
        {
            "key": "radius",
            "label": "采样半径",
            "from": 0.05,
            "to": 3,
            "step": 0.05,
            "decimals": 2
        },
        {
            "key": "bias",
            "label": "深度偏移",
            "from": 0,
            "to": 0.1,
            "step": 0.001,
            "decimals": 3
        },
        {
            "key": "intensity",
            "label": "遮蔽强度",
            "from": 0,
            "to": 3,
            "step": 0.1,
            "decimals": 1
        }
    ]
    // 预留渲染管线阶段及接入说明，这些定义不表示对应 GPU Pass 已执行。
    stages: [
        {
            "title": "深度与法线",
            "description": "提供同一坐标空间的表面位置或深度，以及表面法线。"
        },
        {
            "title": "SSAO",
            "description": "构建半球采样核，将采样点投影到屏幕并比较深度。"
        },
        {
            "title": "降噪",
            "description": "对遮蔽结果进行边缘保持滤波，抑制采样噪声。"
        },
        {
            "title": "环境光合成",
            "description": "将 AO 应用到环境光分量，保留直接光照的贡献。"
        }
    ]
}

