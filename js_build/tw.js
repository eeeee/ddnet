function loadTexture(path, texture, retries) {
  if (retries === null || retries === undefined) {
    retries = 10;
  }
  if (retries <= 0) {
    return;
  }
  var image = new Image();
  image.onload = function() {
    GLctx.bindTexture(GLctx.TEXTURE_2D, GL.textures[texture]);
    GLctx.texImage2D(GLctx.TEXTURE_2D, 0, GLctx.RGBA, GLctx.RGBA, GLctx.UNSIGNED_BYTE, image);

    var powerOf2 = (image.width & (image.width - 1)) == 0 && (image.height & (image.height - 1)) == 0;
    if (powerOf2) {
      GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MAG_FILTER, GLctx.LINEAR);
      GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MIN_FILTER, GLctx.LINEAR_MIPMAP_NEAREST);
      GLctx.generateMipmap(GLctx.TEXTURE_2D);
    } else {
      GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_MIN_FILTER, GLctx.LINEAR);
      GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_WRAP_S, GLctx.CLAMP_TO_EDGE);
      GLctx.texParameteri(GLctx.TEXTURE_2D, GLctx.TEXTURE_WRAP_T, GLctx.CLAMP_TO_EDGE);
    }
    GLctx.bindTexture(GLctx.TEXTURE_2D, null);
  }
  image.onerror = function() {
    setTimeout(function() {
      loadTexture(path, texture, retries - 1);
    }, 1000);
  };
  image.src = "data/" + Pointer_stringify(path);
}

var LibraryTW = {
  loadTexture: loadTexture,
  $loadTexture: loadTexture,
};

autoAddDeps(LibraryTW, '$loadTexture');
mergeInto(LibraryManager.library, LibraryTW);

LibraryManager.library.$Browser.oldXhrLoad = LibraryManager.library.$Browser.xhrLoad;
LibraryManager.library.$Browser.xhrLoad = function() {
  if (Module['xhrLoad']) {
    Module['xhrLoad'].apply(Module, arguments);
  } else {
    Browser.oldXhrLoad.apply(Browser, arguments);
  }
};
