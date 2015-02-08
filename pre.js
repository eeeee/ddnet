Module['loadURL'] = function(url) {
	Module['FS_createPreloadedFile']("/", "demo.demo", url, true, false);
}
Module.noImageDecoding = true;
Module['GL_MAX_TEXTURE_IMAGE_UNITS'] = 1;
Module['getCtx'] = function() { return Module.ctx; };
