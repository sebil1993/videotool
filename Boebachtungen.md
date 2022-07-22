content_length muss genau gesetzt sein : 0 = > kein service 0..n - 1 = > empty response
    n = > antwort
    n++ = > keine antwort kommt zurück, weil er immer noch auf frage wartet

die response liegt entweder als datei oder im buffer vor
=> muss mit pugi geladen werden
  -> datei laden geht
  -> aus buffer auch
    hat daran gelegen, dass die buffergröße zu klein war sizeof(X) => X.length();


Um Namespaces (zumindest für envelope) herauszufinden
    for (pugi::xml_node i : doc.children())
    {
        std::cout << i.name() << std::endl;
        for (pugi::xml_attribute attr : i.attributes())
        {
            if()
            std::cout << attr.name() << std::endl;
        }
    }


namespaces verwerfen, da bspw. tt auch in http steckt u_u
    evtl nach "tt:" suchen?

password digest funktioniert nicht richtig
da unser sha1 ergebnis als hex zurückkommt und nicht als raw binary
    binary => 20 zeichen
    hex =>    40 zeichen
nimmt man aus hex jeweils 2 zeichen erreichen wir die gleiche darstellung (20 zeichen und kauderwelsch)

ich bin jetzt soweit, dass ich bei beiden sha1 (php und c++) auf den gleichen wert komme,
als nächstes muss dieser intepretiert 
und mit base64 verschlüsselt werden 


ayy ich schaffe es die hex in binär umzuwandeln, sollte ich diese nun verketten und base64 kodieren,
so sollte ich den korrekten hash zum einloggen bekommen  



withcitg für zeit implementierung (z.b. die zeit aus gerät verändern) 
https://www.epochconverter.com/programming/c



durch die neue sha1 funktion erreiche ich das gleiche ergebnis beim codieren über c++ wie auch bei php
=> https://github.com/983/SHA1



es gibt probleme wenn man einen veralteten zeitstempel benutzt, der request ist für kurze zeit legit,
jedoch läuft er nach einiger zeit aus und ist dann nicht mehr gültig
=> wenn man zur zeit etwas draufrechnet und dann zu gegebener zeit ausführt
erstmal in c++ machen, dass authentifizierungen gleichen
danach so anpassen dass eine bestimmte zeit hinzugerechnet wird (für AXIS bspw.)
deltatime berechnen: evtl so
DELTATIME = jetziger zeitpunkt - zeitpunkt auf kamera + 5 

es ist möglich sich nonce, passdigest und timestamp für MOBOTIX zu generieren
nächster schritt:
funktion mit authentifizierung ausführen


FEHLER KÖNNEN SICH GANZ LEICHT EINSCHLEUSEN DURCH FEHLERHAFTE SOAP REQUEST 
MOBOTIX kann mehr fehler korrigieren 
AXIS heult rum


DATEIEN ZUSAMMENFÜHREN
in timestamp.cpp 





was funktioniert:
getIso8601 => gibt aktuelle computerzeit zurück im iso format
getsystemdateandtime => gibt die zeit des gerätes zurück

eine curl funktion schreiben und die strings ausgelagert in den funktionen lassen 
je nachdem ob authentifizierung benötigt wird oder nicht 


curl funktion ausgelagert, jetzt gucken wie ich die anderen dinger hineinkriege
funktion -> curl -> neueFunktion(für darstellung der infos)?
                        1. ist envelope, 2. ist 
                        ist es immer das 3. was die anwort besagt?

an sich möchte ich durch die getsystemdateandtime nur die UTC DATE TIME haben, um daraus ein 
passworddigest zu machen mit korrekten zeitstempel


When using Digest authentication, you need to send an HTTP "Authorization" header, and this header is where the nonce etc should go.

However, Digest uses a challenge/response mechanism that requires the Authorization header to be sent in a second HTTP request, rather than in the original HTTP request.

The second HTTP request can only be sent once the original HTTP request has received a 401 response.


ich kann die authentifizierung entweder im soapHEADER regeln oder aber, ich schicke einen Header mit der explizit aussagt wie man sich authentifiziert, bzw. die einloggdaten mit sich führt



gleiches verhalten für axis kamera auf c++:
    mit php gegenprüfen und immer mehr variablen "fest" setzen
    sodass man bei gleicher nonce auf das gleiche ergebnis kommt
    für postman musste ich ein paar sekunden draufrechnen, bevor die axis kamera geantwortet hat
    hier wird das ebenso sein
    evtl anstatt ausführen, auch in postman einfügen und probieren

<Username>seb</Username>
<Password>QVVzTODnloxH7JKOCJsyyKjC1hY=</Password>
<Nonce>MjAyMjA3OTAzMA==</Nonce>
<Created>2022-07-14T14:50:06.000Z</Created> // bisher war aber auch so, dass man einen spielraum von +/- 5 sekunden hatte, dies funktioniert jetzt nur noch bei einer versetzten zeit von -4 bis +4 sekunden





funktioniert soweit, nur das problem ist, dass die funktion getResponselayer (die eigentlich den 3. laer bringt) bei antworten, bei denen ein header mitgeschickt wird hineingeht und stehenbleibt
-> gelöst durch gehen nach "soap-env:body" da das immer gleich ist, selbst bei nem fehler
es kann aber auch vorkommen dass durch das falsch erstellte soap request erst gar keine antwort zurück kommt

eine init() funktion schreiben:
    führt aus GetSystemDateAndTime() -> um die zeit für für das password digest festzulegen, jedoch könnte man es so umschreiben, dass man sich den offset berechnet und diesen auf die systemzeit rechnet, so spart man sich den request bei jedem request
    führt aus GetProfiles() -> um die profilnamen festzusetzen (eventuell überlegen ob man noch die einzelnen infos zu diesen profilen dazu packt)




## MD5
ich würde über curl die snapshot uri periodisch abgreifen und abspeichern (nur bei geräten die snapshot unterstützen)
[MOBOTIX]
Authorization: Digest username="admin",
realm="MegapixelIPCamera", 
nonce="2429ccd96860303ea08556188c38eba7", 
uri="/", 
algorithm=MD5, 
response="4d64e246eedd106df3cb773a2f186e90", 
qop=auth, 
nc=00000001, 
cnonce="ae828c4a84420dcb"'

Digest realm="MegapixelIPCamera", 
qop="auth", 
nonce="5257841dbc1da19ddc55c67fffa2ddff", 
opaque="312-fd0-af5d", 
algorithm="MD5", 
stale="FALSE"

HA1 = MD5(username:realm:password)  // admin:MegapixelIPCamera:password
HA2 = MD5(method:digestURI)         // POST oder GET:/menu.css              die uri wo wir hinwollen
response = MD5(HA1:nonce:HA2)

if qop is 'auth' or 'auth-int'
response = MD5(HA1:nonce:nonceCount:cnonce:qop:HA2)

[wiki](https://en.wikipedia.org/wiki/Digest_access_authentication)



funktion für initial configuration schreiben:
führt standardfunktionen aus und speichert diese ggf. ab (device info, stream uri, snapshot uri, device time)


eventuell neue klasse profil anlegen, in dieser werden die einzelnen einstellungen und stream uris aufgeführt


Datenbank anlegen
- für videodateien
- für profile







[HTTP header line break style](https://stackoverflow.com/questions/5757290/http-header-line-break-style)



ziele für 20.07:
digest muss funktionieren -> dadurch lassen sich die bilder abgreifen, die per http bereitsgestellt werden
bei mobotix funktioniert es ohne probleme
axis nicht:
    könnte cookie fehlen?
    muss man immer die richtige uri mit angeben?
    geht das auf andere art und weise? => "authentication-info:" rspauth...

schlussstrich ziehen/aufhören zu versuchen axis zu digesten (voerst)


-> nächstes bilder über curl abgreifen mit auth von mobotix




WOW curl unterstützt CURLOPT_ANYAUTH d.h. er sucht sich das passende authentifizierungsverfahren, wenn man ihm die richtigen user:pwd gibt


was sind die nächsten schritte?
bilder sekündlich abgreifen, wobei ein bild mit 1920x1080p bereits 300kb hat
500x 300kb /s sind sehr viel an leistung und speicherplatz die erbracht werden muss

[Check virtual memory, RAM, %CPU inside a process](https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process)


[Change FFMPEG parameters on the Fly](https://stackoverflow.com/a/48636290)

[Latenz und Jitterwerte sollten gut sein bei echtzeit](https://www.ibh.de/index/was-bedeutet-latenz-und-jitter)


TODO
    structs erstellen die sich die daten merken, die man beschafft hat
    bspw. videoEncoder{Options}
    daraufhin weiß man welche auflösungen vorliegen, welche encoder
    daraufhin kann man ein pgit helprofil anlegen, über welches man den "abgreif"-link des bildes erstellen (1280x960 z.b. und evtl quality)
    -> so muss man die bilder nicht selbst verkleinern
    //bei axis wäre es so, dass wir nur die parameter im link selbst ändern müssten um das bild zu verkleinern, aber es muss ein viable format gewählt werden


    1. möglichkeit:
        bild periodisch abgreifen über den getsnapshoturi
            muss noch die alten bilder verwerfen, evtl mit deque, dabei schauen ob man die bilder nur im ram speichert und gegen ende verarbeitet oder direkt als dateien hinterlegt, die man periodisch löscht
    2. möglichkeit
        sollte obiges nicht gegeben sein, dann greift man bilder eben periodisch mit einem ffmpeg prozess ab, die frage ist wie viele prozesse kann der ksm handlen in diesem falle
        ich gehe aber davon aus, dass die meisten kameras getsnapshoturi bereitstellen, falls diese in einem unternehmen (etwas mehr gekostet haben/voraussetzung) sind
    wenn das möglich ist
    =>> multiple prozesse mit verschiedenen kameras starten
    
    ======> dafür muss erstmal multithreading verstanden werden    
    
    
    wsdl dateien offline verfügbar machen (wahrscheinlich mit wsdl2h/gsoap++), nicht jedes netzwerk hat internetz
    datenbank erstellen für video und bilder dateien

