# sterne-imgui
Sterne

#### Building
`$ git clone --recurse-submodules -b video https://github.com/doctor-rd/sterne-imgui.git`\
`$ cd sterne-imgui`\
`$ make`\
`$ ./sterne`

#### Video as MP4
`$ ffmpeg -i record.mp4 -c:v copy -video_track_timescale 24 video.mp4`
