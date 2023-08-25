# sterne-imgui
Sterne

#### Building
`$ git clone --recurse-submodules -b video https://github.com/doctor-rd/sterne-imgui.git`\
`$ cd sterne-imgui`\
`$ make`\
`$ ./sterne`

#### Video as MP4
`$ ffmpeg -i record.mp4 -c:v copy -video_track_timescale 24 video.mp4`

#### Troubleshooting
Increase stack size for higher resolutions:
`$ ulimit -s 32768`
