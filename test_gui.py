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
fileEdit = None
inimg = None

def filenameChanged(name):
    print "FILE",name
    inimg = Image.open(name)
    print inimg
    img_shader.in_img(inimg.mode, inimg.size, inimg.tobytes(), 'intex')
    img_shader.render(prog, inimg.size)


def shaderChanged(frag_src):
    if not compileBox.value():
        return
    global prog
    if prog is not None:
        img_shader.del_shader(prog)
    prog = img_shader.compile_frag_shader(frag_src)
    img_shader.render(prog, inimg.size)

def compileChecked(val):
    if val:
        shaderChanged(srcEdit.value())

def inputBrowse():
    name = img_shader.file_dlg("Open", "Open Image File", "Image Files\0*.png;*.jpg;*.tiff\0\0\0")
    if name is not None:
        fileEdit.setValue(name)

def main():
    global compileBox, srcEdit, fileEdit, inimg

    img_shader.init(True)
    
    inimg = Image.open(imgpath)
    print inimg
    img_shader.in_img(inimg.mode, inimg.size, inimg.tobytes(), 'intex')

    #outimgbuf = img_shader.out_img(inimg.mode)
	
    #outimg = Image.frombytes(inimg.mode, inimg.size, outimgbuf)
    #outimg.save(os.path.join(this_dir, "test/out.png"))

    img_shader.create_control_window(400, 600)
    fileEdit = img_shader.create_control("EDIT", 5, 5, 350, 30, imgpath, filenameChanged, resizeMode=('Stretch','None'))
    img_shader.create_control("BUTTON", 360, 5, 35, 30, "...", inputBrowse, resizeMode=('Move', 'None'))
    srcEdit = img_shader.create_control("EDIT", 5, 40, 390, 510, frag_init.replace('\n','\r\n'), shaderChanged, isMultiline=True, resizeMode=('Stretch','Stretch'))
    compileBox = img_shader.create_control("CHECKBOX", 5, 555, 100, 20, "Compile", compileChecked, resizeMode=('None', 'Move'))

    shaderChanged(frag_init)
    img_shader.run_window()

	
	
if __name__ == "__main__":
    main()