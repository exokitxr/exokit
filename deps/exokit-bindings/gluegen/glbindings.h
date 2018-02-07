void __activeTexture(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int texture = (unsigned int)args[0]->IntegerValue();
	glActiveTexture(texture);
}
void __attachShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int program = (unsigned int)args[0]->IntegerValue();
	unsigned int shader = (unsigned int)args[1]->IntegerValue();
	glAttachShader(program,shader);
}
void __bindBuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int buffer = (unsigned int)args[1]->IntegerValue();
	glBindBuffer(target,buffer);
}
void __bindFramebuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int framebuffer = (unsigned int)args[1]->IntegerValue();
	glBindFramebuffer(target,framebuffer);
}
void __bindRenderbuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int renderbuffer = (unsigned int)args[1]->IntegerValue();
	glBindRenderbuffer(target,renderbuffer);
}
void __bindTexture(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int texture = (unsigned int)args[1]->IntegerValue();
	glBindTexture(target,texture);
}
void __blendColor(const v8::FunctionCallbackInfo<v8::Value>& args) {
	khronos_float_t red = (khronos_float_t)args[0]->NumberValue();
	khronos_float_t green = (khronos_float_t)args[1]->NumberValue();
	khronos_float_t blue = (khronos_float_t)args[2]->NumberValue();
	khronos_float_t alpha = (khronos_float_t)args[3]->NumberValue();
	glBlendColor(red,green,blue,alpha);
}
void __blendEquation(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int mode = (unsigned int)args[0]->IntegerValue();
	glBlendEquation(mode);
}
void __blendEquationSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int modeRGB = (unsigned int)args[0]->IntegerValue();
	unsigned int modeAlpha = (unsigned int)args[1]->IntegerValue();
	glBlendEquationSeparate(modeRGB,modeAlpha);
}
void __blendFunc(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int sfactor = (unsigned int)args[0]->IntegerValue();
	unsigned int dfactor = (unsigned int)args[1]->IntegerValue();
	glBlendFunc(sfactor,dfactor);
}
void __blendFuncSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int srcRGB = (unsigned int)args[0]->IntegerValue();
	unsigned int dstRGB = (unsigned int)args[1]->IntegerValue();
	unsigned int srcAlpha = (unsigned int)args[2]->IntegerValue();
	unsigned int dstAlpha = (unsigned int)args[3]->IntegerValue();
	glBlendFuncSeparate(srcRGB,dstRGB,srcAlpha,dstAlpha);
}
void __bufferData(const v8::FunctionCallbackInfo<v8::Value>& args) {
	HandleScope handle_scope(service->GetIsolate());
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	v8::Handle<v8::ArrayBufferView> bufview_data = Handle<ArrayBufferView>::Cast(args[1]);
	v8::Handle<v8::ArrayBuffer> buf_data = bufview_data->Buffer();
	v8::ArrayBuffer::Contents con_data=buf_data->GetContents();
	GLsizeiptr size = con_data.ByteLength();
	void *data = con_data.Data();
	unsigned int usage = (unsigned int)args[2]->IntegerValue();
	glBufferData(target,size,data,usage);
}
void __bufferSubData(const v8::FunctionCallbackInfo<v8::Value>& args) {
	HandleScope handle_scope(service->GetIsolate());
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	khronos_intptr_t offset = (khronos_intptr_t)args[1]->IntegerValue();
	v8::Handle<v8::ArrayBufferView> bufview_data = Handle<ArrayBufferView>::Cast(args[2]);
	v8::Handle<v8::ArrayBuffer> buf_data = bufview_data->Buffer();
	v8::ArrayBuffer::Contents con_data=buf_data->GetContents();
	GLsizeiptr size = con_data.ByteLength();
	void *data = con_data.Data();
	glBufferSubData(target,offset,size,data);
}
void __checkFramebufferStatus(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	GLenum _ret = glCheckFramebufferStatus(target);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), _ret));
}
void __clear(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int mask = (unsigned int)args[0]->IntegerValue();
	glClear(mask);
}
void __clearColor(const v8::FunctionCallbackInfo<v8::Value>& args) {
	khronos_float_t red = (khronos_float_t)args[0]->NumberValue();
	khronos_float_t green = (khronos_float_t)args[1]->NumberValue();
	khronos_float_t blue = (khronos_float_t)args[2]->NumberValue();
	khronos_float_t alpha = (khronos_float_t)args[3]->NumberValue();
	glClearColor(red,green,blue,alpha);
}
void __clearDepthf(const v8::FunctionCallbackInfo<v8::Value>& args) {
	khronos_float_t depth = (khronos_float_t)args[0]->NumberValue();
	glClearDepthf(depth);
}
void __clearStencil(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int s = (int)args[0]->Int32Value();
	glClearStencil(s);
}
void __colorMask(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned char red = (unsigned char)args[0]->Int32Value();
	unsigned char green = (unsigned char)args[1]->Int32Value();
	unsigned char blue = (unsigned char)args[2]->Int32Value();
	unsigned char alpha = (unsigned char)args[3]->Int32Value();
	glColorMask(red,green,blue,alpha);
}
void __compileShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int shader = (unsigned int)args[0]->IntegerValue();
	glCompileShader(shader);
}
void __copyTexImage2D(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	int level = (int)args[1]->Int32Value();
	unsigned int internalformat = (unsigned int)args[2]->IntegerValue();
	int x = (int)args[3]->Int32Value();
	int y = (int)args[4]->Int32Value();
	int width = (int)args[5]->IntegerValue();
	int height = (int)args[6]->IntegerValue();
	int border = (int)args[7]->Int32Value();
	glCopyTexImage2D(target,level,internalformat,x,y,width,height,border);
}
void __copyTexSubImage2D(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	int level = (int)args[1]->Int32Value();
	int xoffset = (int)args[2]->Int32Value();
	int yoffset = (int)args[3]->Int32Value();
	int x = (int)args[4]->Int32Value();
	int y = (int)args[5]->Int32Value();
	int width = (int)args[6]->IntegerValue();
	int height = (int)args[7]->IntegerValue();
	glCopyTexSubImage2D(target,level,xoffset,yoffset,x,y,width,height);
}
void __createProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
	GLuint _ret = glCreateProgram();
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), _ret));
}
void __createShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int type = (unsigned int)args[0]->IntegerValue();
	GLuint _ret = glCreateShader(type);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), _ret));
}
void __cullFace(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int mode = (unsigned int)args[0]->IntegerValue();
	glCullFace(mode);
}
void __deleteProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int program = (unsigned int)args[0]->IntegerValue();
	glDeleteProgram(program);
}
void __deleteShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int shader = (unsigned int)args[0]->IntegerValue();
	glDeleteShader(shader);
}
void __depthFunc(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int func = (unsigned int)args[0]->IntegerValue();
	glDepthFunc(func);
}
void __depthMask(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned char flag = (unsigned char)args[0]->Int32Value();
	glDepthMask(flag);
}
void __depthRangef(const v8::FunctionCallbackInfo<v8::Value>& args) {
	khronos_float_t zNear = (khronos_float_t)args[0]->NumberValue();
	khronos_float_t zFar = (khronos_float_t)args[1]->NumberValue();
	glDepthRangef(zNear,zFar);
}
void __detachShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int program = (unsigned int)args[0]->IntegerValue();
	unsigned int shader = (unsigned int)args[1]->IntegerValue();
	glDetachShader(program,shader);
}
void __disable(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int cap = (unsigned int)args[0]->IntegerValue();
	glDisable(cap);
}
void __disableVertexAttribArray(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int index = (unsigned int)args[0]->IntegerValue();
	glDisableVertexAttribArray(index);
}
void __drawArrays(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int mode = (unsigned int)args[0]->IntegerValue();
	int first = (int)args[1]->Int32Value();
	int count = (int)args[2]->IntegerValue();
	glDrawArrays(mode,first,count);
}
void __drawElements(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int mode = (unsigned int)args[0]->IntegerValue();
	int count = (int)args[1]->IntegerValue();
	unsigned int type = (unsigned int)args[2]->IntegerValue();
	void *indices = (void *)args[3]->IntegerValue();
	glDrawElements(mode,count,type,indices);
}
void __enable(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int cap = (unsigned int)args[0]->IntegerValue();
	glEnable(cap);
}
void __enableVertexAttribArray(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int index = (unsigned int)args[0]->IntegerValue();
	glEnableVertexAttribArray(index);
}
void __finish(const v8::FunctionCallbackInfo<v8::Value>& args) {
	glFinish();
}
void __flush(const v8::FunctionCallbackInfo<v8::Value>& args) {
	glFlush();
}
void __framebufferRenderbuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int attachment = (unsigned int)args[1]->IntegerValue();
	unsigned int renderbuffertarget = (unsigned int)args[2]->IntegerValue();
	unsigned int renderbuffer = (unsigned int)args[3]->IntegerValue();
	glFramebufferRenderbuffer(target,attachment,renderbuffertarget,renderbuffer);
}
void __framebufferTexture2D(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int attachment = (unsigned int)args[1]->IntegerValue();
	unsigned int textarget = (unsigned int)args[2]->IntegerValue();
	unsigned int texture = (unsigned int)args[3]->IntegerValue();
	int level = (int)args[4]->Int32Value();
	glFramebufferTexture2D(target,attachment,textarget,texture,level);
}
void __frontFace(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int mode = (unsigned int)args[0]->IntegerValue();
	glFrontFace(mode);
}
void __generateMipmap(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	glGenerateMipmap(target);
}
void __getAttribLocation(const v8::FunctionCallbackInfo<v8::Value>& args) {
	HandleScope handle_scope(service->GetIsolate());
	unsigned int program = (unsigned int)args[0]->IntegerValue();
	String::Utf8Value _str_name(args[1]->ToString());
	const GLchar *name = *_str_name;
	GLint _ret = glGetAttribLocation(program,name);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), _ret));
}
void __getError(const v8::FunctionCallbackInfo<v8::Value>& args) {
	GLenum _ret = glGetError();
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), _ret));
}
void __getUniformLocation(const v8::FunctionCallbackInfo<v8::Value>& args) {
	HandleScope handle_scope(service->GetIsolate());
	unsigned int program = (unsigned int)args[0]->IntegerValue();
	String::Utf8Value _str_name(args[1]->ToString());
	const GLchar *name = *_str_name;
	GLint _ret = glGetUniformLocation(program,name);
	args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), _ret));
}
void __hint(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int mode = (unsigned int)args[1]->IntegerValue();
	glHint(target,mode);
}
void __isBuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int buffer = (unsigned int)args[0]->IntegerValue();
	GLboolean _ret = glIsBuffer(buffer);
	args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), _ret));
}
void __isEnabled(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int cap = (unsigned int)args[0]->IntegerValue();
	GLboolean _ret = glIsEnabled(cap);
	args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), _ret));
}
void __isFramebuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int framebuffer = (unsigned int)args[0]->IntegerValue();
	GLboolean _ret = glIsFramebuffer(framebuffer);
	args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), _ret));
}
void __isProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int program = (unsigned int)args[0]->IntegerValue();
	GLboolean _ret = glIsProgram(program);
	args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), _ret));
}
void __isRenderbuffer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int renderbuffer = (unsigned int)args[0]->IntegerValue();
	GLboolean _ret = glIsRenderbuffer(renderbuffer);
	args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), _ret));
}
void __isShader(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int shader = (unsigned int)args[0]->IntegerValue();
	GLboolean _ret = glIsShader(shader);
	args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), _ret));
}
void __isTexture(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int texture = (unsigned int)args[0]->IntegerValue();
	GLboolean _ret = glIsTexture(texture);
	args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), _ret));
}
void __lineWidth(const v8::FunctionCallbackInfo<v8::Value>& args) {
	khronos_float_t width = (khronos_float_t)args[0]->NumberValue();
	glLineWidth(width);
}
void __linkProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int program = (unsigned int)args[0]->IntegerValue();
	glLinkProgram(program);
}
void __pixelStorei(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int pname = (unsigned int)args[0]->IntegerValue();
	int param = (int)args[1]->Int32Value();
	glPixelStorei(pname,param);
}
void __polygonOffset(const v8::FunctionCallbackInfo<v8::Value>& args) {
	khronos_float_t factor = (khronos_float_t)args[0]->NumberValue();
	khronos_float_t units = (khronos_float_t)args[1]->NumberValue();
	glPolygonOffset(factor,units);
}
void __releaseShaderCompiler(const v8::FunctionCallbackInfo<v8::Value>& args) {
	glReleaseShaderCompiler();
}
void __renderbufferStorage(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int internalformat = (unsigned int)args[1]->IntegerValue();
	int width = (int)args[2]->IntegerValue();
	int height = (int)args[3]->IntegerValue();
	glRenderbufferStorage(target,internalformat,width,height);
}
void __sampleCoverage(const v8::FunctionCallbackInfo<v8::Value>& args) {
	khronos_float_t value = (khronos_float_t)args[0]->NumberValue();
	unsigned char invert = (unsigned char)args[1]->Int32Value();
	glSampleCoverage(value,invert);
}
void __scissor(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int x = (int)args[0]->Int32Value();
	int y = (int)args[1]->Int32Value();
	int width = (int)args[2]->IntegerValue();
	int height = (int)args[3]->IntegerValue();
	glScissor(x,y,width,height);
}
void __shaderSource(const v8::FunctionCallbackInfo<v8::Value>& args) {
	HandleScope handle_scope(service->GetIsolate());
	unsigned int shader = (unsigned int)args[0]->IntegerValue();
	String::Utf8Value _str_string(args[1]->ToString());
	const GLchar *string[] = {*_str_string};
	GLsizei count = 1;
	GLint length[] = {-1};
	glShaderSource(shader,count,string,length);
}
void __stencilFunc(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int func = (unsigned int)args[0]->IntegerValue();
	int ref = (int)args[1]->Int32Value();
	unsigned int mask = (unsigned int)args[2]->IntegerValue();
	glStencilFunc(func,ref,mask);
}
void __stencilFuncSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int face = (unsigned int)args[0]->IntegerValue();
	unsigned int func = (unsigned int)args[1]->IntegerValue();
	int ref = (int)args[2]->Int32Value();
	unsigned int mask = (unsigned int)args[3]->IntegerValue();
	glStencilFuncSeparate(face,func,ref,mask);
}
void __stencilMask(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int mask = (unsigned int)args[0]->IntegerValue();
	glStencilMask(mask);
}
void __stencilMaskSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int face = (unsigned int)args[0]->IntegerValue();
	unsigned int mask = (unsigned int)args[1]->IntegerValue();
	glStencilMaskSeparate(face,mask);
}
void __stencilOp(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int fail = (unsigned int)args[0]->IntegerValue();
	unsigned int zfail = (unsigned int)args[1]->IntegerValue();
	unsigned int zpass = (unsigned int)args[2]->IntegerValue();
	glStencilOp(fail,zfail,zpass);
}
void __stencilOpSeparate(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int face = (unsigned int)args[0]->IntegerValue();
	unsigned int fail = (unsigned int)args[1]->IntegerValue();
	unsigned int zfail = (unsigned int)args[2]->IntegerValue();
	unsigned int zpass = (unsigned int)args[3]->IntegerValue();
	glStencilOpSeparate(face,fail,zfail,zpass);
}
void __texParameterf(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int pname = (unsigned int)args[1]->IntegerValue();
	khronos_float_t param = (khronos_float_t)args[2]->NumberValue();
	glTexParameterf(target,pname,param);
}
void __texParameteri(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int target = (unsigned int)args[0]->IntegerValue();
	unsigned int pname = (unsigned int)args[1]->IntegerValue();
	int param = (int)args[2]->Int32Value();
	glTexParameteri(target,pname,param);
}
void __uniform1f(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int location = (int)args[0]->Int32Value();
	khronos_float_t x = (khronos_float_t)args[1]->NumberValue();
	glUniform1f(location,x);
}
void __uniform1fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,1,1);
}
void __uniform1i(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int location = (int)args[0]->Int32Value();
	int x = (int)args[1]->Int32Value();
	glUniform1i(location,x);
}
void __uniform1iv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,1,0);
}
void __uniform2f(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int location = (int)args[0]->Int32Value();
	khronos_float_t x = (khronos_float_t)args[1]->NumberValue();
	khronos_float_t y = (khronos_float_t)args[2]->NumberValue();
	glUniform2f(location,x,y);
}
void __uniform2fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,2,1);
}
void __uniform2i(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int location = (int)args[0]->Int32Value();
	int x = (int)args[1]->Int32Value();
	int y = (int)args[2]->Int32Value();
	glUniform2i(location,x,y);
}
void __uniform2iv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,2,0);
}
void __uniform3f(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int location = (int)args[0]->Int32Value();
	khronos_float_t x = (khronos_float_t)args[1]->NumberValue();
	khronos_float_t y = (khronos_float_t)args[2]->NumberValue();
	khronos_float_t z = (khronos_float_t)args[3]->NumberValue();
	glUniform3f(location,x,y,z);
}
void __uniform3fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,3,1);
}
void __uniform3i(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int location = (int)args[0]->Int32Value();
	int x = (int)args[1]->Int32Value();
	int y = (int)args[2]->Int32Value();
	int z = (int)args[3]->Int32Value();
	glUniform3i(location,x,y,z);
}
void __uniform3iv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,3,0);
}
void __uniform4f(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int location = (int)args[0]->Int32Value();
	khronos_float_t x = (khronos_float_t)args[1]->NumberValue();
	khronos_float_t y = (khronos_float_t)args[2]->NumberValue();
	khronos_float_t z = (khronos_float_t)args[3]->NumberValue();
	khronos_float_t w = (khronos_float_t)args[4]->NumberValue();
	glUniform4f(location,x,y,z,w);
}
void __uniform4fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,4,1);
}
void __uniform4i(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int location = (int)args[0]->Int32Value();
	int x = (int)args[1]->Int32Value();
	int y = (int)args[2]->Int32Value();
	int z = (int)args[3]->Int32Value();
	int w = (int)args[4]->Int32Value();
	glUniform4i(location,x,y,z,w);
}
void __uniform4iv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,4,0);
}
void __uniformMatrix2fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,2,2);
}
void __uniformMatrix3fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,3,2);
}
void __uniformMatrix4fv(const v8::FunctionCallbackInfo<v8::Value>& args) {
	__uniformv(args,4,2);
}
void __useProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int program = (unsigned int)args[0]->IntegerValue();
	glUseProgram(program);
}
void __validateProgram(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int program = (unsigned int)args[0]->IntegerValue();
	glValidateProgram(program);
}
void __vertexAttrib1f(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int indx = (unsigned int)args[0]->IntegerValue();
	khronos_float_t x = (khronos_float_t)args[1]->NumberValue();
	glVertexAttrib1f(indx,x);
}
void __vertexAttrib2f(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int indx = (unsigned int)args[0]->IntegerValue();
	khronos_float_t x = (khronos_float_t)args[1]->NumberValue();
	khronos_float_t y = (khronos_float_t)args[2]->NumberValue();
	glVertexAttrib2f(indx,x,y);
}
void __vertexAttrib3f(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int indx = (unsigned int)args[0]->IntegerValue();
	khronos_float_t x = (khronos_float_t)args[1]->NumberValue();
	khronos_float_t y = (khronos_float_t)args[2]->NumberValue();
	khronos_float_t z = (khronos_float_t)args[3]->NumberValue();
	glVertexAttrib3f(indx,x,y,z);
}
void __vertexAttrib4f(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int indx = (unsigned int)args[0]->IntegerValue();
	khronos_float_t x = (khronos_float_t)args[1]->NumberValue();
	khronos_float_t y = (khronos_float_t)args[2]->NumberValue();
	khronos_float_t z = (khronos_float_t)args[3]->NumberValue();
	khronos_float_t w = (khronos_float_t)args[4]->NumberValue();
	glVertexAttrib4f(indx,x,y,z,w);
}
void __vertexAttribPointer(const v8::FunctionCallbackInfo<v8::Value>& args) {
	unsigned int indx = (unsigned int)args[0]->IntegerValue();
	int size = (int)args[1]->Int32Value();
	unsigned int type = (unsigned int)args[2]->IntegerValue();
	unsigned char normalized = (unsigned char)args[3]->Int32Value();
	int stride = (int)args[4]->IntegerValue();
	void *ptr = (void *)args[5]->IntegerValue();
	glVertexAttribPointer(indx,size,type,normalized,stride,ptr);
}
void __viewport(const v8::FunctionCallbackInfo<v8::Value>& args) {
	int x = (int)args[0]->Int32Value();
	int y = (int)args[1]->Int32Value();
	int width = (int)args[2]->IntegerValue();
	int height = (int)args[3]->IntegerValue();
	glViewport(x,y,width,height);
}
