The Click
=========

I have attempted to measure the ear *click* noise using an oscilloscope
connected to the output of the amplifier circuit on the RECAP instrumentation
board. This was unsuccessful. I tried placing the microphone inside the ear, on
on the bone infront of the ear, behind the ear, and no click would be picked up
on the scope. I am unsure as to why not.

I have looked at some literature around this and have noted the exact impulse I
was expecting [1]. That is, a sharp impulse leading to a 4-5 kHz decaying
sinusoid. The paper doesn't document their measurement technique but I
understand they are using a microphone, likely inserted further into the ear
canal than anything I can do. This leads me to the following suggestions on how
to get this working properly:

 - Try a better microphone type.
 - Use a microphone designed to be inserted into the ear canal with a seal, much
   like how ear buds work (perhaps look at using an ear bud chassis to house a
   microphone).
 - Use an analog band-pass filter centered on 5 kHz with a small pass region.
 - Once the click is detectable, use analog circuitry to generate a "click/no
   click" digital signal which can be sampled by the SAM4S without the need for
   software filtering.




References
----------

1 - "Physiological" Ear Clicking - It's origin and potential usability as a test
tool for the eustachian tube function.