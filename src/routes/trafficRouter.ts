import { Router } from 'express';
import { TrafficController } from '../controllers/trafficController';

export const trafficRouter = Router();
const trafficController = new TrafficController();

// POST /api/traffic - Create new traffic data
trafficRouter.post('/', trafficController.createTraffic);

// GET /api/traffic - Get all traffic data
trafficRouter.get('/', trafficController.getAllTraffic);

// GET /api/traffic/latest - Get latest traffic data
trafficRouter.get('/latest', trafficController.getLatestTraffic);

// GET /api/traffic/:id - Get traffic data by ID
trafficRouter.get('/:id', trafficController.getTrafficById);

// PUT /api/traffic/:id - Update traffic data by ID
trafficRouter.put('/:id', trafficController.updateTraffic);

// DELETE /api/traffic/:id - Delete traffic data by ID
trafficRouter.delete('/:id', trafficController.deleteTraffic);