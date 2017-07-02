import sys, os
from PIL import Image

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append( os.path.join(this_dir, "Release_x64" ))

import img_shader

frag_src = """
uniform sampler2D intex;
varying vec2  pos;
void main(void) {
    vec2 p = pos;
    vec4 v = texture2D(intex, p);
    
    gl_FragColor = vec4(v.r, v.g, v.b, v.a);
}
"""

def main():
    inimg = Image.open( os.path.join(this_dir, "test/gray_test.png") )

 #   inimg = Image.open( os.path.join(this_dir, "test/rgba_test3.png") )
    print inimg
    inimgstr = inimg.tobytes()
    
    img_shader.init(True, inimg.size[0], inimg.size[1])
    prog = img_shader.compile_frag_shader(frag_src)
    
    tex = img_shader.in_img(inimg.mode, inimgstr)
  
    img_shader.render(prog, tex)
    outimgbuf = img_shader.out_img(inimg.mode)
	
    outimg = Image.frombytes(inimg.mode, inimg.size, outimgbuf)
    outimg.save(os.path.join(this_dir, "test/out.png"))

    img_shader.run_window()
    
    diff = 0
    assert( len(inimgstr) == len(outimgbuf))
    for i in xrange(len(inimgstr)):
        d = abs(ord(inimgstr[i]) - ord(outimgbuf[i]))
        diff += d
    print "diff=", diff
	
	
if __name__ == "__main__":
    main()