import { Request, Response } from 'express';
import { PredictionService } from '../services/predictionService';

export class PredictionController {
    private predictionService: PredictionService;

    constructor() {
        this.predictionService = new PredictionService();
    }

    createPrediction = async (req: Request, res: Response): Promise<void> => {
        try {
            const predictionData = req.body;
            const prediction = await this.predictionService.createPrediction(predictionData);
            res.status(201).json({
                success: true,
                message: 'Prediction created successfully',
                data: prediction
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error creating prediction',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    getAllPredictions = async (req: Request, res: Response): Promise<void> => {
        try {
            const predictions = await this.predictionService.getAllPredictions();
            res.status(200).json({
                success: true,
                message: 'Predictions retrieved successfully',
                data: predictions
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error retrieving predictions',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    getPredictionById = async (req: Request, res: Response): Promise<void> => {
        try {
            const id = parseInt(req.params.id);
            const prediction = await this.predictionService.getPredictionById(id);
            
            if (!prediction) {
                res.status(404).json({
                    success: false,
                    message: 'Prediction not found'
                });
                return;
            }

            res.status(200).json({
                success: true,
                message: 'Prediction retrieved successfully',
                data: prediction
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error retrieving prediction',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    getPredictionByTrafficIntensities = async (req: Request, res: Response): Promise<void> => {
        try {
            const trafficIntensity1 = parseInt(req.params.trafficIntensity1);
            const trafficIntensity2 = parseInt(req.params.trafficIntensity2);
            const prediction = await this.predictionService.getPredictionByTrafficIntensities(trafficIntensity1, trafficIntensity2);
            
            if (!prediction) {
                res.status(404).json({
                    success: false,
                    message: 'Prediction not found'
                });
                return;
            }

            res.status(200).json({
                success: true,
                message: 'Prediction retrieved successfully',
                data: prediction
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error retrieving prediction',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    updatePrediction = async (req: Request, res: Response): Promise<void> => {
        try {
            const id = parseInt(req.params.id);
            const predictionData = req.body;
            const prediction = await this.predictionService.updatePrediction(id, predictionData);
            
            if (!prediction) {
                res.status(404).json({
                    success: false,
                    message: 'Prediction not found'
                });
                return;
            }

            res.status(200).json({
                success: true,
                message: 'Prediction updated successfully',
                data: prediction
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error updating prediction',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    deletePrediction = async (req: Request, res: Response): Promise<void> => {
        try {
            const id = parseInt(req.params.id);
            const deleted = await this.predictionService.deletePrediction(id);
            
            if (!deleted) {
                res.status(404).json({
                    success: false,
                    message: 'Prediction not found'
                });
                return;
            }

            res.status(200).json({
                success: true,
                message: 'Prediction deleted successfully'
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error deleting prediction',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    getLatestPrediction = async (req: Request, res: Response): Promise<void> => {
        try {
            const prediction = await this.predictionService.getLatestPrediction();
            
            if (!prediction) {
                res.status(404).json({
                    success: false,
                    message: 'No predictions found'
                });
                return;
            }

            res.status(200).json({
                success: true,
                message: 'Latest prediction retrieved successfully',
                data: prediction
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error retrieving latest prediction',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };
}