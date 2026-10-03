function @mode() -> i64 [binding=weak, inline_hint=yes] {
  block ^entry:
    return i64 0
}
function @get(%p : ptr, %index : i64, %size : i64) -> i64 [binding=weak, inline_hint=yes] {
  block ^entry:
    %checked = call i64 @mode()
    branch %checked, ^checked, ^ordinary
  block ^checked:
    %divided = binary div i64 %index, %size
    return i64 %divided
  block ^ordinary:
    %address = index i64 %p, %index
    %value = load i64 %address
    return i64 %value
}
function @sum(%p : ptr, %count : i64) -> i64 [binding=strong] {
  block ^entry:
    jump ^loop
  block ^loop:
    %index = phi i64 [^entry: 0, ^body: %next]
    %total = phi i64 [^entry: 0, ^body: %added]
    %more = cmp ult i64 %index, %count
    branch %more, ^body, ^done
  block ^body:
    %value = call i64 @get(%p, %index, %count)
    %added = binary add i64 %total, %value
    %next = binary add i64 %index, 1
    jump ^loop
  block ^done:
    return i64 %total
}
