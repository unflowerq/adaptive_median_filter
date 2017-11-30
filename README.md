# adaptive_median_filter

median_filter보다 한 단계 발전된 필터이다.

필터를 사용하기에 앞서서 마스크의 최대값을 지정한다.

- 𝑍_𝑚𝑒𝑑 : 마스크 내의 픽셀들 중 중간값
- 𝑍_𝑚𝑖𝑛 : 마스크 내의 픽셀들 중 최소값
- 𝑍_𝑀𝑎𝑥 : 마스크 내의 픽셀들 중 최대값
- 𝑆_𝑀𝑎𝑥 : 마스크의 최대값
- 𝑍_𝑋𝑌 : 현재 위치의 값


Level A

- A1 = 𝑍_𝑚𝑒𝑑 − 𝑍_𝑚𝑖𝑛
- A2 = 𝑍_𝑚𝑒𝑑 − 𝑍_𝑀𝑎𝑥
- if A1 > 0 and A2 < 0, Go to Level B
- else Increase mask size
- if mask size < 𝑆_𝑀𝑎𝑥, repeat Level A
- else return 𝑍_𝑋𝑌

Level B

- B1 = 𝑍_𝑋𝑌 − 𝑍_𝑚𝑖𝑛
- B2 = 𝑍_𝑋𝑌 − 𝑍_𝑀𝑎𝑥
- if B1 > 0 and B2 < 0, return 𝑍_𝑋𝑌
- else return 𝑍_𝑚𝑒𝑑
