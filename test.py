import sys, os
from PIL import Image

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append( os.path.join(this_dir, "Release_x64" ))

import img_shader

frag_src = """
uniform sampler2D intex;
uniform sampler2D in2;
varying vec2  pos;
void main(void) {
    vec2 p = pos;
    vec4 v = texture2D(intex, p);
    vec4 v2= texture2D(in2, p);
    
    gl_FragColor = vec4(v2.r, v.r, v.r, v.a);
}
"""

def main():
    inimg = Image.open( os.path.join(this_dir, "test/gray_test.png") )

 #   inimg = Image.open( os.path.join(this_dir, "test/rgba_test3.png") )
    print inimg
    inimgstr = inimg.tobytes()
    
    img_shader.init(True, inimg.size[0], inimg.size[1])
    prog = img_shader.compile_frag_shader(frag_src)
    
    img_shader.in_img(inimg.mode, inimgstr, "intex")
    img_shader.in_img(inimg.mode, inimgstr, "in2")

    img_shader.render(prog)
    outimgbuf = img_shader.out_img(inimg.mode)
	
    outimg = Image.frombytes(inimg.mode, inimg.size, outimgbuf)
    outimg.save(os.path.join(this_dir, "test/out.png"))

    img_shader.run_window()

    check_diff(inimgstr, outimgbuf)


def check_diff(img1, img2):
    diff = 0
    assert( len(img1) == len(img2))
    for i in xrange(len(img1)):
        d = abs(ord(img1[i]) - ord(img2[i]))
        diff += d
    print "diff=", diff

	
if __name__ == "__main__":
    main()