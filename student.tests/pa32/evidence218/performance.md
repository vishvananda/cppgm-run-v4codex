# Audit 218 frozen measurements

Ratios are paired B/A medians with all six paired extrema in brackets. Times are separate sample medians; RSS is the maximum in the measured ABBA blocks. Every JSON retains the A/A calibration, all individual observations, flags, inputs, checked exit statuses and compiler/object/executable hashes.

Affected baselines precede their owning implementation; B is the final reviewed compiler. Common controls compare entry/final at the same level. Scalar compares final O0/O1. Debug compares final O3 g0/line-table mode and measures the cost of required snapshots. The self-hosting component has no executable entry. These are diagnostics interpreted under the stage acceptance in [audit.md](../../../pa32/audit.md).

## common-o0

[All samples](common-o0.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| memory | 0.938 [0.733–1.096] | 323.12/304.39 | 30880/30884 | 0.950 [0.833–1.232] | 99.20/106.72 | 151393/151393 |
| floating | 0.982 [0.817–1.013] | 315.09/308.55 | 31016/31024 | 0.988 [0.933–1.064] | 80.56/81.14 | 151234/151234 |
| exceptions | 1.045 [0.784–1.291] | 343.99/344.54 | 30752/30768 | 1.025 [0.814–1.616] | 492.64/507.75 | 151541/151541 |
| pruning | 0.988 [0.690–1.186] | 570.81/586.33 | 36380/36436 | 1.003 [0.971–1.185] | 98.99/98.68 | 151393/151393 |

## common-o1

[All samples](common-o1.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| memory | 0.987 [0.938–1.021] | 477.45/467.94 | 30740/30856 | 0.949 [0.810–1.057] | 127.86/115.80 | 124959/124959 |
| floating | 1.017 [0.986–1.136] | 230.51/229.86 | 30948/31036 | 1.000 [0.946–1.019] | 50.16/50.62 | 124856/124856 |
| exceptions | 1.032 [0.617–1.134] | 237.23/250.79 | 30352/30348 | 1.011 [0.995–1.026] | 252.96/255.09 | 125053/125053 |
| pruning | 1.009 [0.785–1.180] | 252.33/255.12 | 36440/36472 | 0.998 [0.932–1.102] | 71.57/73.28 | 124959/124959 |

## common-o3

[All samples](common-o3.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| memory | 0.993 [0.835–1.170] | 350.16/348.70 | 30756/30704 | 1.001 [0.993–2.281] | 49.04/49.05 | 124959/124959 |
| floating | 1.006 [0.997–1.304] | 213.43/214.62 | 31036/30904 | 1.000 [0.994–1.005] | 48.64/48.63 | 124856/124856 |
| exceptions | 0.996 [0.918–1.187] | 236.85/294.02 | 30316/30352 | 0.992 [0.987–1.003] | 253.64/250.57 | 125053/125053 |
| pruning | 1.001 [0.526–1.005] | 242.25/242.49 | 36376/36488 | 1.001 [0.995–1.057] | 48.81/48.90 | 124959/124959 |

## selfhost

[All samples](selfhost.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| component | 0.992 [0.937–1.018] | 1094.46/1096.67 | 77896/77836 | N/A | N/A | 34466/34466 |

## scalar

[All samples](scalar.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| kernel | 2.431 [2.397–2.506] | 51.89/126.84 | 10212/15404 | 0.873 [0.867–0.883] | 75.23/65.66 | 170400/109800 |

## objects

[All samples](objects.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| copies | 1.055 [1.031–1.064] | 150.15/158.84 | 17884/20872 | 0.511 [0.511–0.513] | 357.80/183.02 | 192000/144000 |
| export | 1.151 [1.092–1.161] | 129.85/149.84 | 17924/19416 | 0.600 [0.597–0.602] | 355.39/213.04 | 188428/159628 |

## loops

[All samples](loops.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| finite | 0.862 [0.852–0.876] | 90.83/78.28 | 12748/11960 | 0.123 [0.122–0.123] | 1145.31/140.57 | 128000/28800 |
| unroll | 1.166 [1.142–1.209] | 125.71/147.14 | 16228/17920 | 0.453 [0.450–0.465] | 378.54/171.11 | 153600/151481 |

## memory

[All samples](memory.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| loads | 1.032 [1.022–1.068] | 243.28/254.89 | 32432/28512 | 0.582 [0.575–0.597] | 145.36/84.75 | 111600/117000 |
| conditional | 1.040 [1.025–1.094] | 118.72/125.33 | 15916/16240 | 1.000 [0.990–1.010] | 82.60/82.61 | 147600/147600 |
| private | 0.946 [0.761–1.013] | 130.50/124.90 | 18396/15092 | 0.988 [0.980–1.015] | 85.90/84.61 | 122400/127800 |
| private-loads | 1.060 [1.030–1.151] | 148.77/157.54 | 18500/19036 | 0.991 [0.941–1.004] | 85.53/85.48 | 135000/135000 |
| copies | 0.887 [0.862–0.904] | 103.34/91.03 | 17224/15852 | 1.004 [0.940–1.016] | 82.47/82.64 | 66600/66600 |
| diamonds | 0.885 [0.856–0.936] | 305.37/270.32 | 29112/26236 | 0.929 [0.886–0.983] | 105.74/98.25 | 340200/207000 |

## ranges

[All samples](ranges.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| reference-large | 0.954 [0.948–1.064] | 335.81/321.43 | 35468/31316 | 0.010 [0.009–0.015] | 7820.21/75.32 | 440000/420009 |
| reference-small | 0.955 [0.853–0.966] | 339.91/323.02 | 35464/31176 | 0.797 [0.788–0.801] | 1061.72/847.15 | 440000/420009 |
| reference-empty | 0.962 [0.940–1.049] | 335.43/320.47 | 35480/31276 | 0.945 [0.935–0.954] | 190.41/179.73 | 440000/420009 |
| zero-large | 0.951 [0.924–0.962] | 345.34/329.71 | 33596/33692 | 0.042 [0.042–0.043] | 1885.18/79.16 | 405000/355009 |
| pointer | 0.786 [0.771–0.905] | 270.92/212.80 | 31584/27676 | 0.038 [0.037–0.038] | 4496.42/172.15 | 360000/70000 |
| truth | 0.860 [0.822–0.924] | 390.23/325.63 | 34316/34792 | 0.780 [0.768–0.986] | 614.05/477.36 | 465000/260000 |
| partial | 1.060 [0.860–1.272] | 478.44/490.60 | 31444/31532 | 0.972 [0.740–1.010] | 493.31/486.28 | 400000/400000 |

## context

[All samples](context.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| checked | 2.472 [2.376–2.611] | 39.86/98.73 | 9028/20504 | 0.491 [0.413–0.526] | 135.26/67.28 | 50535/36000 |
| floating | 0.505 [0.482–0.547] | 112.38/59.42 | 16092/13336 | 0.956 [0.848–0.972] | 75.23/72.41 | 228000/33600 |
| parent | 1.377 [1.277–1.406] | 361.14/461.85 | 31784/39280 | 0.204 [0.185–0.240] | 358.66/73.38 | 168046/40846 |
| landing | 1.060 [1.047–1.139] | 240.98/252.99 | 19560/19756 | 1.010 [0.844–1.029] | 392.21/395.51 | 156071/150068 |

## source

[All samples](source.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| layout | 0.864 [0.809–0.910] | 169.66/150.18 | 24192/22612 | 0.781 [0.679–0.905] | 95.45/78.53 | 66099/16865 |
| move | 0.972 [0.943–1.005] | 443.44/426.95 | 38236/38556 | 0.994 [0.893–1.006] | 219.96/218.54 | 98510/98510 |
| landing | 0.965 [0.913–0.985] | 363.40/353.07 | 40508/40904 | 0.952 [0.894–1.018] | 228.61/212.14 | 170012/164040 |

## debug

[All samples](debug.json)

| Workload | Compiler ratio [spread] | Compile ms A/B | Compiler RSS KiB A/B | Runtime ratio [spread] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| kernel | 1.065 [0.766–1.148] | 415.16/456.04 | 26528/25476 | 1.356 [1.210–1.513] | 246.01/327.10 | 129856/129600 |
