import sys, os
from PIL import Image

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append( os.path.join(this_dir, "Debug_x64" ))

import img_shader

frag_src = """
uniform sampler2D intex;
varying vec2  pos;
void main(void) {
    vec2 p = pos;
    vec4 v = texture2D(intex, p);
    
    gl_FragColor = vec4(v.r, v.r, v.r, 1.0);
    //gl_FragColor = vec4(p.x, p.y, 1.0, 1.0);
}
"""

def main():
    img = Image.open( os.path.join(this_dir, "test/gray_test3.png") )
  #  assert(img.mode == 'L') # check it's a grayscale image
   
 #   img = Image.open( os.path.join(this_dir, "test/rgba_test3.png") )
    print img
    imgstr = img.tostring()
    
    
    img_shader.set_img_size(img.size[0], img.size[1])
    img_shader.init()
    prog = img_shader.compile_frag_shader(frag_src)
    
    tex = img_shader.in_grayscale_byte(imgstr) 
  #  tex = img_shader.in_rgba_byte(imgstr)
  
    img_shader.render(prog, tex)
    img_shader.run_window()

if __name__ == "__main__":
    main()