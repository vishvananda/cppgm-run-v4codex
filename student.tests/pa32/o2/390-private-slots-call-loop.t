declare function @observe(%x : i64) -> i64 [unwind=no]
function @sum(%count : i64) -> i64 [binding=strong] {
  slot $index : i64
  slot $total : i64
  block ^entry:
    store i64 0, $index
    store i64 0, $total
    jump ^loop
  block ^loop:
    %index = load i64 $index
    %more = cmp ult i64 %index, %count
    branch %more, ^body, ^done
  block ^body:
    %value = call i64 @observe(%index)
    %total = load i64 $total
    %added = binary add i64 %total, %value
    store i64 %added, $total
    %next = binary add i64 %index, 1
    store i64 %next, $index
    jump ^loop
  block ^done:
    %result = load i64 $total
    return i64 %result
}
