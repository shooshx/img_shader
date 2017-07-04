import sys, os
from PIL import Image

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append( os.path.join(this_dir, "Debug_x64" ))

import img_shader

frag_init = """
uniform sampler2D intex;
varying vec2  pos;
void main(void) {
    vec2 p = pos;
    vec4 v = texture2D(intex, p);
    
    gl_FragColor = vec4(v.r, v.g, v.b, v.a);
}
"""

imgpath = os.path.join(this_dir, "test/gray_test.png")
#imgpath = os.path.join(this_dir, "test/rgba_test3.png")


prog = None
compileBox = None
srcEdit = None

def filenameChanged(name):
    print name

def shaderChanged(frag_src):
    if not compileBox.value():
        return
    global prog
    if prog is not None:
        img_shader.del_shader(prog)
    prog = img_shader.compile_frag_shader(frag_src)
    img_shader.render(prog)

def compileChecked(val):
    if val:
        try:
            shaderChanged(srcEdit.value())
        except Exception as e:
            print e
def inputBrowse():
    print "browse"

def main():
    global compileBox, srcEdit
    inimg = Image.open(imgpath)

    print inimg
    inimgstr = inimg.tobytes()
    
    img_shader.init(True, inimg.size[0], inimg.size[1])
    
    img_shader.in_img(inimg.mode, inimgstr, 'intex')

    #outimgbuf = img_shader.out_img(inimg.mode)
	
    #outimg = Image.frombytes(inimg.mode, inimg.size, outimgbuf)
    #outimg.save(os.path.join(this_dir, "test/out.png"))

    img_shader.create_control_window(400, 600)
    img_shader.create_control("EDIT", 5, 5, 350, 30, imgpath, filenameChanged, resizeMode=('Stretch','None'))
    img_shader.create_control("BUTTON", 360, 5, 35, 30, "...", inputBrowse, resizeMode=('Move', 'None'))
    srcEdit = img_shader.create_control("EDIT", 5, 40, 390, 510, frag_init.replace('\n','\r\n'), shaderChanged, isMultiline=True, resizeMode=('Stretch','Stretch'))
    compileBox = img_shader.create_control("CHECKBOX", 5, 555, 100, 20, "Compile", compileChecked, resizeMode=('None', 'Move'))

    shaderChanged(frag_init)
    img_shader.run_window()

	
	
if __name__ == "__main__":
    main()