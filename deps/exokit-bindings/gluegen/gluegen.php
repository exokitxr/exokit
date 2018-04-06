<?php

// gl.h lends itself well to shallow line-base parsing.  We look for:
//   #define <name> <value>
// or
//   GL_API [returntype] GL_APIENTRY gl[func] ( [params] ) ;

$GLOBJ = "_gl";

$js_returnclause = array(
	"void" => "void",
);

$js_getparam = array(
	"GLenum" => "IntegerValue",
	"GLboolean" => "Int32Value",
	"GLbitfield" => "IntegerValue",
	"GLint" => "Int32Value",
	"GLsizei" => "IntegerValue",
	"GLubyte" => "Int32Value",
	"GLuint" => "IntegerValue",
	"GLfloat" => "NumberValue",
	"GLclampf" => "NumberValue",
	"GLfixed" => "NumberValue",
	"GLclampx" => "Int32Value",
	"GLintptr" => "IntegerValue",
	"GLsizeiptr" => "IntegerValue",
);

$js_paramtype = array(
	"GLenum" => "unsigned int",
	"GLboolean" => "unsigned char",
	"GLbitfield" => "unsigned int",
	"GLint" => "int",
	"GLsizei" => "int",
	"GLubyte" => "khronos_uint8_t",
	"GLuint" => "unsigned int",
	"GLfloat" => "khronos_float_t",
	"GLclampf" => "khronos_float_t",
	"GLfixed" => "khronos_int32_t",
	"GLclampx" => "khronos_int32_t",
	"GLintptr" => "khronos_intptr_t",
	"GLsizeiptr" => "khronos_ssize_t",
);



$FUNC="";

$STAT="";

/*

Special cases.


Void pointer functions.

size, data => array



GL_API void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);

GL_API void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);


HTML Image element:


GL_API void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);


not supported yet:


GL_API void GL_APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);


GL_API void GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);


GL_API void GL_APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);


GL_API void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);

GL_API void GL_APIENTRY glGetPointerv (GLenum pname, GLvoid **params);


GL_API void GL_APIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer);


GL_API void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);


GL_API void GL_APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);





GL_API void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);


GL_API void GL_APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);


GL_API void GL_APIENTRY glPointSizePointerOES (GLenum type, GLsizei stride, const GLvoid *pointer);


*/


/* // how to get values
	double d = args[0]->NumberValue();
	float f = (float)args[0]->NumberValue();
	int32_t g = args[1]->Int32Value();
	int64_t g = args[1]->IntegerValue();
	bool b = args[1]->BooleanValue();
	// array index
	float i[1] = args[0]->ToObject()->Get(1);

// example gl function call
void __clearColor(const v8::FunctionCallbackInfo<v8::Value>& args) {
	LOGI("Enter clearcolor");
	if (args.Length() < 4) return; // error -> return undefined
	float r = (float)args[0]->NumberValue();
	float g = (float)args[1]->NumberValue();
	float b = (float)args[2]->NumberValue();
	float a = (float)args[3]->NumberValue();
    glClearColor(r,g,b,a);
	return;
	// Wrap the result in a JavaScript string and return it.
	//args.GetReturnValue().Set(String::NewFromUtf8(
	//	args.GetIsolate(), path.c_str(), String::kNormalString,
	//	static_cast<int>(path.length())));
}


*/




function genJSValue($name,$value) {
	global $GLOBJ;
	global $FUNC,$STAT;
	$STAT .= "\t".$GLOBJ."->Set(v8::String::NewFromUtf8(isolate,\"$name\"), v8::Integer::New(isolate,$value));\n";
}

function genJSFunction($retval,$funcname,$funcparams) {
	global $GLOBJ,$js_getparam,$js_paramtype,$js_returnclause;
	global $FUNC,$STAT;
	$skip = false; // skip entire function
	$skipcall = false; // skip call to GL (if already generated)
	$need_scope = false; // true if HandleScope needs to be created
	$FH = ""; // function header
	$FB = ""; // function body
	$S = ""; // init call to create function
	// return type
	// functionname
	$S.="\t${GLOBJ}->Set(v8::String::NewFromUtf8(isolate, \"".lcfirst($funcname)
		."\"), v8::FunctionTemplate::New(isolate, __"
		.lcfirst($funcname)."));\n";
	$FH.="void __".lcfirst($funcname)."(const v8::FunctionCallbackInfo<v8::Value>& args) {\n";
	// get parameters
	$pars = explode(",",$funcparams);
	$types = array();
	$names = array();
	$nameidxs = array();
	$pointerpos = array();
	$pos = 0;
	foreach ($pars as $par) {
		if (preg_match("/^\s*([^ ].*\W)(\w+)\s*$/",$par,$res)) {
			$name = $res[2];
			$type = trim($res[1]);
			$type = preg_replace("/const\s+/","",$type);
			$types[] = $type;
			$names[] = $name;
			$nameidxs[] = $pos;
			if (strpos($type,"*")!==false) {
				// pointer type -> store pos
				$pointerpos[] = $pos;
			}
		} else if (trim($par) == "void") {
			// empty parameter list
		} else {
			// unknown parameter format
			echo "##################UNKNOWN PAR: $par\n";
		}
		$pos++;
	}
	// process parameters
	if (sizeof($pointerpos)> 0) {
		$skip=true;
		// rewrite parameter list or skip
		if ($funcname=="ShaderSource") {
			$skip=false;
			$types = array("GLuint","_StringArray");
			$nameidxs = array(0,1);
		} else if ($funcname=="DrawElements") {
			//GL_APICALL void         GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
			$skip=false;
			$types[3] = "_IntPtr";
		} else if ($funcname=="BufferData") {
			$skip=false;
			//GL_APICALL void         GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
			$types = array("GLenum","_BufferData","GLenum");
			$nameidxs = array(0,1,3);
		} else if ($funcname=="BufferSubData") {
			$skip=false;
			//GL_APICALL void         GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
			$types = array("GLenum","GLintptr","_BufferData");
			$nameidxs = array(0,1,2);
		} else if ($funcname=="VertexAttribPointer") {
			//GL_APICALL void         GL_APIENTRY glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
			$skip=false;
			$types[5]="_IntPtr";
		} else if ($funcname=="GetUniformLocation"
		||         $funcname=="GetAttribLocation") {
			$skip=false;
			$types[1] = "_String";
		} else if(preg_match('/Uniform(.*)([1234])([fi])v/',$funcname,$unipar)){
			$skip=false;
			$skipcall=true; // we generate internal call instead of direct call
			$ismatrix = $unipar[1] == "Matrix";
			$vecsize = $unipar[2];
			$vectype = $unipar[3];
			$vectype = $ismatrix ? 2 : ($vectype=="i" ? 0 : 1);
			$types = array("_UniformParam","_UniformParam","_UniformParam",
				"_UniformParam");
			$FB.="\t__uniformv(args,$vecsize,$vectype);\n";
		}

	}
	if (!$skip) {
		// no pointers -> convert normally
		for ($i=0; $i<sizeof($nameidxs); $i++) {
			$ctype = $types[$i];
			$idx = $nameidxs[$i];
			$cname = $names[$idx];
			if ($ctype == "_String") {
				$need_scope = true;
				$FB.="\tString::Utf8Value _str_$cname(args[$i]->ToString());\n";
				$FB.="\tconst GLchar *$cname = *_str_$cname;\n";
			} else if ($ctype == "_StringArray") {
				// special case: 1 element long string array
				$need_scope = true;
				$countname = $names[$idx];
				$strname = $names[$idx+1];
				$lenname = $names[$idx+2];
				$FB.="\tString::Utf8Value _str_$strname(args[$i]->ToString());\n";
				$FB.="\tconst GLchar *{$strname}[] = {*_str_$strname};\n";
				$FB.="\tGLsizei $countname = 1;\n";
				$FB.="\tGLint {$lenname}[] = {-1};\n";
			} else if ($ctype == "_BufferData") {
				$need_scope = true;
				$lenname = $names[$idx];
				$bufname = $names[$idx+1];
				// Taken from node-threads/web-worker.cc
				//https://github.com/inh3/node-threads/blob/master/src/node-threads/web-worker.cc#L9
				// Apparently the value may be either an arraybuffer or
				// arraybufferview. In both cases it can be safely cast to
				// arraybufferview.  Casting to arraybuffer may cause failure.
				$FB.="\tv8::Handle<v8::ArrayBufferView> bufview_$bufname = Handle<ArrayBufferView>::Cast(args[$i]);\n";
				
				$FB.="\tv8::Handle<v8::ArrayBuffer> buf_$bufname = bufview_{$bufname}->Buffer();\n";
				$FB.="\tv8::ArrayBuffer::Contents con_$bufname=buf_{$bufname}->GetContents();\n";
				$FB.="\tGLsizeiptr $lenname = con_{$bufname}.ByteLength();\n";
				$FB.="\tvoid *$bufname = con_{$bufname}.Data();\n";
			} else if ($ctype == "_IntPtr") {
				$FB.="\tvoid *$cname = (void *)args[$i]->IntegerValue();\n";
			} else if ($ctype == "_UniformParam") {
				// resulted in call to other function, do nothing here
			} else {
				$ctype = $js_paramtype[$types[$i]];
				$jsget = $js_getparam[$types[$i]];
				$FB.="\t$ctype $cname = ($ctype)args[$i]->$jsget();\n";
			}
		}
		// function call
		if (!$skipcall) {
			$FB.="\t";
			if ($retval!="void") {
				$FB.="$retval _ret = ";
			}
			$FB.="gl$funcname(".implode(",",$names).");\n";
			if ($retval=="GLboolean") {
				//$need_scope = true;
				$FB.="\targs.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), _ret));\n";
			} else if ($retval!="void") { // only ints found until now
				//$need_scope = true;
			
				$FB.="\targs.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), _ret));\n";
				// Wrap the result in a JavaScript string and return it.
				//args.GetReturnValue().Set(String::NewFromUtf8(
				//	args.GetIsolate(), path.c_str(), String::kNormalString,
				//	static_cast<int>(path.length())));
			}
		}
		$FB.="}\n";
		if ($need_scope) {
			$FB = "\tHandleScope handle_scope(service->GetIsolate());\n" . $FB;
		}
		$FUNC .= $FH . $FB;
		$STAT .= $S;
	} else {
		echo "SKIPPED: $funcname($funcparams)\n";
	}
}



$f = fopen( 'gl2-android9-arm.h', 'r' );
//$f = fopen( 'php://stdin', 'r' );

while( $line = fgets( $f ) ) {
	if (preg_match("/^\s*#define/",$line)) {
		if (preg_match("/^\s*#define\s+GL_(\w+)\s+(\w+)/",$line,$param)) {
			// #define <name> <value>
			//echo "DEFINE($param[1],$param[2]);\n";
			genJSValue($param[1],$param[2]);
		} else {
			// check for #defines that do not match
			echo "########NOT MATCHED: $line";
		}
	} else if (preg_match("/GL_API[CAL]*\s+(\w+)\s+GL_APIENTRY/",$line)) {
		if (preg_match(
"/GL_API[CAL]*\s+(\w+)\s+GL_APIENTRY\s+gl(\w+)\s*[(]\s*([^)]+)\s*[)]\s*;/",$line,$param)) {
			// GL_API [returntype] GL_APIENTRY [func] ( [params] ) ;
			//echo "GLAPI($param[1],$param[2],$param[3]);\n";
			genJSFunction($param[1],$param[2],$param[3]);

		} else {
			// check for #defines that do not match
			echo "##########NOT MATCHED: $line";
		}
	}
}

fclose( $f );

file_put_contents("glbindings.h",$FUNC);

file_put_contents("glbindinit.h",$STAT);


?>
