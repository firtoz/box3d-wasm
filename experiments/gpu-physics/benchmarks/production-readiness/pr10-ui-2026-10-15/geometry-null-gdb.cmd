set pagination off
set confirm off
set debuginfod enabled off
set print thread-events off
break b3CreateHull
commands
silent
if $esi == 8
printf "PR10CREATE point_count=%d max_vertices=%d\n", $esi, $edx
x/24fw $rdi
end
continue
end
break DrawHull if $rdi == 0
commands
silent
printf "PR10NULLHULL before original DrawHull dereference\n"
bt 6
info registers rdi
quit
end
run
