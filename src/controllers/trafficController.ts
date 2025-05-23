import { Request, Response } from 'express';
import { TrafficService } from '../services/trafficService';

export class TrafficController {
    private trafficService: TrafficService;

    constructor() {
        this.trafficService = new TrafficService();
    }

    createTraffic = async (req: Request, res: Response): Promise<void> => {
        try {
            const trafficData = req.body;
            const traffic = await this.trafficService.createTraffic(trafficData);
            res.status(201).json({
                success: true,
                message: 'Traffic data created successfully',
                data: traffic
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error creating traffic data',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    getAllTraffic = async (req: Request, res: Response): Promise<void> => {
        try {
            const traffic = await this.trafficService.getAllTraffic();
            res.status(200).json({
                success: true,
                message: 'Traffic data retrieved successfully',
                data: traffic
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error retrieving traffic data',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    getTrafficById = async (req: Request, res: Response): Promise<void> => {
        try {
            const id = parseInt(req.params.id);
            const traffic = await this.trafficService.getTrafficById(id);
            
            if (!traffic) {
                res.status(404).json({
                    success: false,
                    message: 'Traffic data not found'
                });
                return;
            }

            res.status(200).json({
                success: true,
                message: 'Traffic data retrieved successfully',
                data: traffic
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error retrieving traffic data',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    updateTraffic = async (req: Request, res: Response): Promise<void> => {
        try {
            const id = parseInt(req.params.id);
            const trafficData = req.body;
            const traffic = await this.trafficService.updateTraffic(id, trafficData);
            
            if (!traffic) {
                res.status(404).json({
                    success: false,
                    message: 'Traffic data not found'
                });
                return;
            }

            res.status(200).json({
                success: true,
                message: 'Traffic data updated successfully',
                data: traffic
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error updating traffic data',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    deleteTraffic = async (req: Request, res: Response): Promise<void> => {
        try {
            const id = parseInt(req.params.id);
            const deleted = await this.trafficService.deleteTraffic(id);
            
            if (!deleted) {
                res.status(404).json({
                    success: false,
                    message: 'Traffic data not found'
                });
                return;
            }

            res.status(200).json({
                success: true,
                message: 'Traffic data deleted successfully'
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error deleting traffic data',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };

    getLatestTraffic = async (req: Request, res: Response): Promise<void> => {
        try {
            const traffic = await this.trafficService.getLatestTraffic();
            
            if (!traffic) {
                res.status(404).json({
                    success: false,
                    message: 'No traffic data found'
                });
                return;
            }

            res.status(200).json({
                success: true,
                message: 'Latest traffic data retrieved successfully',
                data: traffic
            });
        } catch (error) {
            res.status(500).json({
                success: false,
                message: 'Error retrieving latest traffic data',
                error: error instanceof Error ? error.message : 'Unknown error'
            });
        }
    };
}