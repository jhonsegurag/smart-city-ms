import { Router } from 'express';
import { PredictionController } from '../controllers/predictionController';

export const predictionRouter = Router();
const predictionController = new PredictionController();

// POST /api/predictions - Create new prediction
predictionRouter.post('/', predictionController.createPrediction);

// GET /api/predictions - Get all predictions
predictionRouter.get('/', predictionController.getAllPredictions);

// GET /api/predictions/latest - Get latest prediction
predictionRouter.get('/latest', predictionController.getLatestPrediction);

// GET /api/predictions/:id - Get prediction by ID
predictionRouter.get('/:id', predictionController.getPredictionById);

// GET /api/predictions/:id - Get prediction by ID
predictionRouter.get('/:trafficIntensity1/:trafficIntensity2', predictionController.getPredictionByTrafficIntensities);

// PUT /api/predictions/:id - Update prediction by ID
predictionRouter.put('/:id', predictionController.updatePrediction);

// DELETE /api/predictions/:id - Delete prediction by ID
predictionRouter.delete('/:id', predictionController.deletePrediction);