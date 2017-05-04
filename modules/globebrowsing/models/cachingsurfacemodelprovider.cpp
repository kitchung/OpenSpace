/*****************************************************************************************
*                                                                                       *
* OpenSpace                                                                             *
*                                                                                       *
* Copyright (c) 2014-2017                                                               *
*                                                                                       *
* Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
* software and associated documentation files (the "Software"), to deal in the Software *
* without restriction, including without limitation the rights to use, copy, modify,    *
* merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
* permit persons to whom the Software is furnished to do so, subject to the following   *
* conditions:                                                                           *
*                                                                                       *
* The above copyright notice and this permission notice shall be included in all copies *
* or substantial portions of the Software.                                              *
*                                                                                       *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
* PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
* OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
****************************************************************************************/

#include <modules/globebrowsing/models/cachingsurfacemodelprovider.h>
#include <modules/globebrowsing/other/threadpool.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/io/texture/texturereader.h>

namespace {
	const std::string _loggerCat = "CachingSurfaceModelProvider";
}

namespace openspace {
namespace globebrowsing {

CachingSurfaceModelProvider::CachingSurfaceModelProvider(Renderable* parent)
	: _parent(parent)
{
	double cacheSize = 512;

	auto threadPool = std::make_shared<ThreadPool>(1);
	_asyncSurfaceModelProvider = std::make_shared<AsyncSurfaceModelProvider>(threadPool);
	_modelCache = std::make_shared<ModelCache>(static_cast<size_t>(cacheSize));
}

std::vector<std::shared_ptr<Model>> CachingSurfaceModelProvider::getModels(const ghoul::Dictionary& dictionary,
	std::shared_ptr<Model> model) {

	// Check if the chunk is already in the cache. If it is in the cache, loop through the corresponding vector
	// and look if the model is already loaded. If the model is already loaded, return the vector. If the model is
	// not already loaded, enqueue the model.
	std::vector<std::shared_ptr<Model>> models;
	bool modelIsInCache = true;
	if (_modelCache->exist(model->tileHashKey)) {
		models = _modelCache->get(model->tileHashKey);

		for (auto i : models) {
			std::string tempName = i->fileName;
			if (i->fileName == model->fileName) {
				return models;
			}
		}
		modelIsInCache = false;
		if(!modelIsInCache) {
			_asyncSurfaceModelProvider->enqueueModelIO(dictionary, model);
		}
	}
	else {
		_asyncSurfaceModelProvider->enqueueModelIO(dictionary, model);
	}

	return models;
}

void CachingSurfaceModelProvider::update(Renderable* parent) {
	initModelsFromLoadedData(parent);
}

void CachingSurfaceModelProvider::initModelsFromLoadedData(Renderable* parent) {
	std::vector<std::shared_ptr<Model>> models =
		_asyncSurfaceModelProvider->getLoadedModels();

	for (auto model : models) {
		model->geometry->initialize(parent);
		
		// Must create a new texture because the texture created in the load job 
		// is given the id zero (0). In this way the texture gets a unique id.
		void* pixelData = new char[model->texture->expectedPixelDataSize()];
		memcpy(pixelData, model->texture->pixelData(), model->texture->expectedPixelDataSize());

		std::shared_ptr<ghoul::opengl::Texture> tempTexture = std::make_shared<ghoul::opengl::Texture>(
			pixelData,
			model->texture->dimensions(),
			model->texture->format(),
			model->texture->internalFormat(),
			model->texture->dataType(),
			model->texture->filter(),
			model->texture->wrapping()
			);

		model->texture = tempTexture;

		// Upoad to GPU and set filter
		model->texture->uploadTexture();
		model->texture->setFilter(ghoul::opengl::Texture::FilterMode::AnisotropicMipMap);

		uint64_t hashKey = model->tileHashKey;

		if (_modelCache->exist(hashKey)){
			std::vector<std::shared_ptr<Model>> tempModels = _modelCache->get(hashKey);
			tempModels.push_back(model);
			_modelCache->put(hashKey, tempModels);
		}
		else {
			std::vector<std::shared_ptr<Model>> tempVector;
			tempVector.push_back(model);
			_modelCache->put(model->tileHashKey, tempVector);
		}
	}
}



} // namespace globebrowsing
} // namespace openspace