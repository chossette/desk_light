from colorsys import hsv_to_rgb
from math import factorial
import colorsys
import random
import matplotlib.pyplot as plt
from perlin_noise import PerlinNoise
from matplotlib.patches import Rectangle

from tkinter import Tk, Canvas

# input: hue color
# input: dim
# output: for each pixel:
#            dim factor
#            saturation factor

# shift value
# dim factor
def foo1():
    noise = PerlinNoise(octaves=10, seed=1)
    xpix, ypix = 100, 100
    pic = [[noise([i/xpix, j/ypix]) for j in range(xpix)] for i in range(ypix)]
    plt.imshow(pic, cmap='gray')
    plt.show()

def foo2():
    noise1 = PerlinNoise(octaves=3)
    noise2 = PerlinNoise(octaves=6)
    noise3 = PerlinNoise(octaves=12)
    noise4 = PerlinNoise(octaves=24)

    xpix, ypix = 100, 100
    pic = []
    for i in range(xpix):
        row = []
        for j in range(ypix):
            noise_val = noise1([i/xpix, j/ypix])
            noise_val += 0.5 * noise2([i/xpix, j/ypix])
            noise_val += 0.25 * noise3([i/xpix, j/ypix])
            noise_val += 0.125 * noise4([i/xpix, j/ypix])

            row.append(noise_val)
        pic.append(row)

    plt.imshow(pic, cmap='gray')
    plt.show()

def foo3():
    someX, someY = 0.5, 0.5
    plt.figure()
    currentAxis = plt.gca()
    noiseSat = PerlinNoise(octaves=10, seed=1)
    noiseDim = PerlinNoise(octaves=24, seed=1)
    iValue = 0
    led_count = 35
    led_height = 1/led_count
    while True:
        iValue = (iValue + 1) % 100
        for led in range(led_count):
            nsat = noiseSat([led/led_count, iValue/100])
            ndim = noiseDim([led/led_count, iValue/100])
            print("s: {} v:{}".format(nsat, ndim))
            satValue = min((nsat + 1), 1)
            dimValue = min((ndim + 1), 1)
            currentAxis.add_patch(Rectangle((0.5 - .1, led_height * led), 0.2, led_height, alpha=1, color=colorsys.hsv_to_rgb(60/360, satValue, dimValue)))
        plt.pause(0.01)


def foo4():
    root = Tk()
    canvas_height=100
    canvas_width=600
    cnv = Canvas(root, width=600, height=canvas_height)
    cnv.pack()
    led_count = 35
    led_height = canvas_height / led_count
    
    noiseSat = PerlinNoise(octaves=10, seed=1)
    noiseDim = PerlinNoise(octaves=24, seed=1)


    led_array = [(60/360, 1.0, 1.0) for i in range(led_count)]
    heat_array = [100 for i in range(led_count)] 

    def htmlcolor(r, g, b):
        return "#{:02x}{:02x}{:02x}".format(int(r), int(g), int(b))
    
    def update(larray, iValue, cnv, nsat, ndim):
        cnv.delete("all")
        lwidth = int(cnv.cget('width'))
        lheight = int(cnv.cget('height'))
        led_count = len(larray)
        led_height = lheight / led_count

        for led in range(led_count):
            iValue = (iValue + led) % 100
            # perlinNoise => value entre -1 et 1 
            #             => adding percent of the current value
            #             => adding between -100% and +100% to the current value
            h, s, v = larray[led]
            incFactor = 0.1
            sFactor = nsat([led/led_count, iValue / 100])
            dFactor = ndim([led/led_count, iValue / 100])
            sRes = max(min(s + (s * sFactor) * incFactor, 1.0), 0.)
            vRes = max(min(v + (v * dFactor) * incFactor, 1.0), 0.)
            
            r, g, b = colorsys.hsv_to_rgb(h, sRes, vRes)
            cnv.create_rectangle(0, led * led_height, lwidth, (led+1) * led_height, fill=htmlcolor(int(r*255), int(g*255), int(b*255)))
        cnv.after(100, update, larray, iValue, cnv, nsat, ndim)

    def update2(larray, harray, cnv):
        cnv.delete("all")
        lwidth = int(cnv.cget('width'))
        lheight = int(cnv.cget('height'))
        led_count = len(larray)
        led_height = lheight
        led_width = lwidth / led_count
        
        cooling_speed = 3
        sparkle_threshold = 40
        sparkle_heat_min = 80

        # 1. cool down leds
        # 2. diffuse from left and right leds
        # 3. sparkle some
        # 4. draw
        
        def heat_color(hc):
            max_light = 191
            # scale to [0; max_light[
            hs = hc * max_light / 100
            # hotests
            if hs > 2*max_light/3: # hotest, nearly white
                return (255, 255, hs)
            elif hs > max_light/3: # medium, nearly yellow
                return (255, hs, 0)
            # cold, nearly red
            return (hs, 0, 0)

        # draw
        for led in range(led_count):
            # 1. cool down
            heat = max(harray[led] - random.randint(0, cooling_speed), 0)
            # 2. diffuse from left and right leds
            lled = led_count - 1 if led == 0 else led - 1
            rled = (led + 1) % led_count
            heat = (harray[lled] + harray[rled] + heat) / 3
            # 3. sparkle !
            if heat < 50 and random.randint(0, 100) > sparkle_threshold:
                heat = random.randint(sparkle_heat_min, 100)

            harray[led] = heat
            # 4. draw
            h, s, v = larray[led]
            if heat > 80: 
                h = h + 0.1 if h <= 0.8 else h + 0.1 - 1.0
            elif heat < 50: 
                h = h - 0.1 if h >= 0.1 else h + 1.0 - 0.1
            rs, gs, bs = colorsys.hsv_to_rgb(h, s, heat/100)
            r, g, b = ( rs * 255, gs * 255, bs * 255)
            
            cnv.create_rectangle(led * led_width, 0, (led+1) * led_width, led_height, fill=htmlcolor(r, g, b))
        cnv.after(20, update2, larray, harray, cnv)
        

    
    # update(led_array, 0, cnv, noiseSat, noiseDim)
    update2(led_array, heat_array, cnv)
    root.mainloop()
    

    
foo4()