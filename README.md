# DIY mechanical weed control 

## history

In autumn of 2020 a friend asks a friend of mine and me:
> "You two are into computers, right?
Can you program a software which uses a camera to look at the plants on a field and control my self-build hydraulic shifted harrow?" 
I replied: "Are you crazy?"

So the journey begins. I was very sceptical and thought we had not the slightest chance to get anything running.
We didn't even know where to start.

2021 - Spring

But my friend doesn't give up and hacked some prototype in Python using OpenCV.
We did a test out in the fields using a broomstick which we centered over one row of plants. Using a Raspi4 and a shitty webcam.
The software should center the stick above the plants as the tractor moves left or right.
To my absolute astonishment it works not that bad!!
I catched fire! Thanks to my friend not giving up in the first place! :-)

2021 - Summer

It was thinking, guessing, trying, thinking again, talking to other people.
Porting our prototype to C++.
Came up with the idea of "reference lines".

2022

I coded the idea of reference lines.
We're testing my prototype code with green dots that we painted on some cardboard. Moving the carboard underneath the camera/harrow to make it move accordingly. Fun times! 

Our friend the farmer reserved a small field for us for testing. He planted some sunflowers if I remember correctly.
When the sunflowers grow in April we gave our prototype the first run.
It wasn't that bad. I destroyed maybe 10 to 15 meters of plants. :-)
But the rest of the field was done correctly.

I think we "hacked" 100 hectars in 2022.

to be continued...

## Software - build, run

## docker (easy)

    1. docker build -t traktor .
    2. docker run --rm --device=/dev/video0 -p 9080:9080 traktor

With an camera as /dev/video0 the Web UI should now be reachable by: http://localhost:9080

## build on your own

We develop and run our system on a Raspberry 4.
You can use any type of computer, but controlling is right now implemented via GPIO.

### prerequistites

    sudo apt install libopencv-dev libgpiod-dev

    + mkdir -p ./deps/cpp-httplib ./deps/json
    + wget -O ./deps/cpp-httplib/httplib.h https://github.com/yhirose/cpp-httplib/raw/master/httplib.h
    + wget -O ./deps/json/json.hpp https://github.com/nlohmann/json/raw/develop/single_include/nlohmann/json.hpp

### build

    mkdir Release
    cd Release
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make

### run
    ./traktor
