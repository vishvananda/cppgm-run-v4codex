# Frozen compiler and runtime measurements

B/A paired medians, with the range of all six ABBA ratios in brackets. Compiler and runtime wall times are separate sample medians; compiler RSS is the maximum in paired blocks. Every input, compiler and checked image has a retained hash. A/A calibration and every observation remain in the linked JSON. No course timing gate is introduced.

## affected

[All observations](affected.json)

| Workload | Compiler B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| loop | 0.996 [0.779–1.143] | 165.92/170.58 | 12380/12760 | 0.348 [0.336–0.498] | 288.74/100.06 | 127200/123600 |
| calls | 1.033 [0.945–1.123] | 157.28/162.37 | 14508/14884 | 0.786 [0.710–0.919] | 597.90/478.88 | 140414/136814 |
| short-string | 1.012 [0.646–1.046] | 60.71/60.06 | 7804/8240 | 0.558 [0.331–0.600] | 321.92/184.69 | 32000/38784 |
| long-string | 1.016 [0.672–1.305] | 91.90/109.98 | 7796/8204 | 1.122 [1.039–1.231] | 352.50/403.36 | 32000/38784 |
| dynamic-copy | 1.028 [0.968–1.105] | 75.03/80.36 | 8304/8728 | 0.032 [0.029–0.070] | 8140.11/236.60 | 46114/46101 |

## rejected-suffix

[All observations](rejected-suffix.json)

| Workload | Compiler B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| short-string | 1.001 [0.993–1.326] | 48.78/48.91 | 7852/8244 | 0.589 [0.575–0.600] | 199.07/117.64 | 32000/41088 |
| long-string | 1.002 [0.969–1.071] | 31.03/30.94 | 7828/8284 | 1.340 [1.228–1.475] | 233.20/320.58 | 32000/41088 |

## suffix-ab

[All observations](suffix-ab.json)

| Workload | Compiler B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| long-string | 1.000 [0.991–1.238] | 31.29/31.27 | 7704/7772 | 1.389 [1.363–1.396] | 251.43/350.92 | 38784/41088 |

## common-o0

[All observations](common-o0.json)

| Workload | Compiler B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| memory | 1.001 [0.970–1.228] | 281.40/279.76 | 30072/30168 | 1.002 [0.986–1.048] | 71.23/71.04 | 151393/151393 |
| floating | 1.014 [0.997–1.196] | 277.45/283.92 | 30660/30992 | 1.005 [0.971–1.086] | 58.03/58.18 | 151234/151234 |
| exceptions | 0.998 [0.950–1.039] | 233.31/233.78 | 30256/30412 | 1.018 [0.943–1.261] | 366.80/451.85 | 151541/151541 |
| pruning | 1.007 [0.932–1.258] | 383.39/376.95 | 36056/36376 | 0.913 [0.386–1.004] | 82.68/78.45 | 151393/151393 |

## common-o2

[All observations](common-o2.json)

| Workload | Compiler B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| memory | 0.978 [0.641–1.032] | 557.60/558.46 | 29964/30180 | 0.991 [0.983–1.019] | 95.37/94.70 | 124959/124939 |
| floating | 0.996 [0.956–1.436] | 418.43/438.11 | 30712/30772 | 0.888 [0.856–0.924] | 90.69/77.95 | 124856/124846 |
| exceptions | 0.978 [0.752–1.203] | 423.62/410.50 | 30056/30416 | 0.933 [0.745–1.128] | 457.66/458.09 | 125053/125028 |
| pruning | 0.956 [0.904–1.351] | 917.46/896.52 | 36064/36368 | 0.982 [0.931–1.059] | 114.57/111.69 | 124959/124939 |

## selfhost

[All observations](selfhost.json)

| Workload | Compiler B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| component | 0.955 [0.718–1.012] | 2086.55/1910.97 | 77624/77888 | N/A | N/A | 34466/34466 |
