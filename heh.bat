main -h 240 -w 320 -s 4 --animate 1 --anim_position_start "15 5 2" --anim_position_end "15 5 2" --anim_steps 20 --anim_aperture_start 0.05 --anim_aperture_end 0.25 --anim_focus_start 10 --anim_focus_end 10

echo Do compile those?
pause

"C:\Program Files\FFMPEGx64\bin\ffmpeg.exe" -i animation\out_%%02d.ppm -c:v libvpx -qmin 0 -qmax 53 -crf 7.0 -b:v 4M -cpu-used 0 -quality good -framerate 1/23 -r 23 out_proc.webm
