# Final frozen performance evidence

A/A calibration followed by six wall-time ABBA blocks; paired B/A medians and all paired extrema. Compilation and checked execution are separate. CPU 2. Every observation, RSS value, input/binary hash and A/A range is retained in the linked JSON. A is the stage base except scratch, where A is the audited entry (219). B is the final audited compiler.

## affected

[All samples](affected.json)

| Workload | Compile B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| loop | 1.007 [0.979–1.046] | 95.77/94.98 | 12448/12692 | 0.335 [0.317–0.350] | 186.57/62.00 | 127200/123600 |
| calls | 1.015 [0.931–1.025] | 94.57/96.96 | 14496/14976 | 0.635 [0.628–0.639] | 182.48/115.73 | 140414/136814 |
| short-string | 1.014 [1.001–1.017] | 32.19/32.54 | 7828/8204 | 0.595 [0.589–0.597] | 201.24/119.15 | 32000/38784 |
| long-string | 1.007 [0.982–1.045] | 32.18/32.35 | 7820/8248 | 1.140 [1.103–1.202] | 232.34/260.02 | 32000/38784 |
| dynamic-copy | 1.012 [0.997–1.015] | 34.96/35.25 | 8304/8664 | 0.029 [0.028–0.031] | 8324.81/237.26 | 46114/46101 |

## common-o0

[All samples](common-o0.json)

| Workload | Compile B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| memory | 1.002 [0.893–1.144] | 172.71/172.82 | 29880/30060 | 1.004 [0.953–1.036] | 70.46/71.27 | 151393/151393 |
| floating | 1.050 [0.997–1.218] | 177.76/240.98 | 30604/30776 | 1.004 [0.994–1.015] | 56.05/56.52 | 151234/151234 |
| exceptions | 1.005 [0.777–1.122] | 274.99/273.30 | 30248/30304 | 0.969 [0.955–1.048] | 460.67/452.82 | 151541/151541 |
| pruning | 1.092 [0.955–1.200] | 235.39/261.40 | 36084/36168 | 0.982 [0.841–1.076] | 56.85/55.62 | 151393/151393 |

## common-o2

[All samples](common-o2.json)

| Workload | Compile B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| memory | 1.010 [0.952–1.147] | 226.37/231.61 | 30072/30076 | 0.970 [0.815–1.056] | 52.52/52.74 | 124959/124939 |
| floating | 0.970 [0.840–1.015] | 337.38/329.44 | 30652/30792 | 0.913 [0.880–1.194] | 50.60/46.06 | 124856/124846 |
| exceptions | 1.050 [0.983–1.164] | 347.75/357.16 | 30220/30280 | 1.038 [0.906–1.391] | 289.53/310.35 | 125053/125028 |
| pruning | 0.956 [0.864–1.184] | 290.12/295.49 | 36204/36184 | 0.923 [0.881–1.012] | 58.56/54.20 | 124959/124939 |

## selfhost

[All samples](selfhost.json)

| Workload | Compile B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| selfhost | 0.820 [0.751–0.945] | 1618.60/1321.51 | 77592/77820 | N/A | N/A | 34466/34466 |

## scratch

[All samples](scratch.json)

| Workload | Compile B/A [range] | Compile ms A/B | Peak KiB A/B | Runtime B/A [range] | Runtime ms A/B | Object text A/B |
| --- | --- | --- | --- | --- | --- | --- |
| scratch | 0.831 [0.719–1.065] | 563.78/568.12 | 39848/39668 | 0.985 [0.847–1.082] | 243.00/238.98 | 1290240/1290240 |
