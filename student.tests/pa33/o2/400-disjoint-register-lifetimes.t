global @seen : i64 = 0
global @saved : i64 = 0
function @observe(%value : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %next = binary add i64 %old, 1
    store i64 %next, @seen
    return i64 %value
}
function @main() -> i64 [role=entry] {
  block ^entry:
    jump ^first
  block ^first:
    %i = phi i64 [^entry: 0, ^first_body: %next_i]
    %a = phi i64 [^entry: 0, ^first_body: %next_a]
    %b = phi i64 [^entry: 0, ^first_body: %next_b]
    %more_i = cmp ult i64 %i, 10
    branch %more_i, ^first_body, ^between
  block ^first_body:
    %x = call i64 @observe(%i)
    %twice = binary mul i64 %x, 2
    %next_a = binary add i64 %a, %x
    %next_b = binary add i64 %b, %twice
    %next_i = binary add i64 %i, 1
    jump ^first
  block ^between:
    %first_sum = binary add i64 %a, %b
    store i64 %first_sum, @saved
    jump ^second
  block ^second:
    %j = phi i64 [^between: 0, ^second_body: %next_j]
    %c = phi i64 [^between: 0, ^second_body: %next_c]
    %d = phi i64 [^between: 0, ^second_body: %next_d]
    %more_j = cmp ult i64 %j, 10
    branch %more_j, ^second_body, ^done
  block ^second_body:
    %y = call i64 @observe(%j)
    %four = binary mul i64 %y, 4
    %next_c = binary add i64 %c, %y
    %next_d = binary add i64 %d, %four
    %next_j = binary add i64 %j, 1
    jump ^second
  block ^done:
    %earlier = load i64 @saved
    %last = binary add i64 %c, %d
    %total = binary add i64 %earlier, %last
    %wrong = cmp ne i64 %total, 360
    branch %wrong, ^bad, ^check_calls
  block ^check_calls:
    %calls = load i64 @seen
    %bad_calls = cmp ne i64 %calls, 20
    return i64 %bad_calls
  block ^bad:
    return i64 1
}
