
checkt die keyframes

ffprobe -loglevel error -select_streams v:0 -show_entries packet=pts_time,flags -of csv=print_section=0 rtsp://10.15.100.200/jpeg | awk -F',' '/K/ {print $1}'


behält alle segmente, nimmt aber nur die letzten beiden in die playlist auf

ffmpeg -i rtsp://10.15.100.200/jpeg -c:v h264 -flags +cgop -g 30 -hls_time 1 -hls_list_size 2 out.m3u8

cat `cat out.m3u8 | grep .ts` | > hihi.ts
playlist lesen, grep "out*.ts", in datei schreiben, daten aus datei concat      
=>
fertige ts datei            

cat `ls | grep .ts` | > hihi.ts   
lese stattdessen ordner aus

sekündlich bild abgreifen ffmpeg
ffmpeg -i 'rtsp://seb:sebseb@10.15.2.201/onvif-media/media.amp?profile=profile_1_h264&sessiontimeout=60&streamtype=unicast' -r 1 'img-%03d.jpeg'


nimmt aus der playlist alle .ts dateien, concated diese dateien zu einer und schickt sie dann in einen ffpmeg prozess zur konvertierung
cat `cat low1fps.m3u8 | grep .ts` | > lowconcat.ts && ffmpeg -y -i lowconcat.ts -c:v libx264 lowconcatandmp4.mp4



