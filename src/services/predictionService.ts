import { AppDataSource } from '../datasource/datasource';
import { PredictionEntity } from '../entity/predictionEntity';
import { Between, Repository } from 'typeorm';

export class PredictionService {
    private predictionRepository: Repository<PredictionEntity>;

    constructor() {
        this.predictionRepository = AppDataSource.getRepository(PredictionEntity);
    }

    async createPrediction(predictionData: Partial<PredictionEntity>): Promise<PredictionEntity> {
        const prediction = this.predictionRepository.create(predictionData);
        return await this.predictionRepository.save(prediction);
    }

    async getAllPredictions(): Promise<PredictionEntity[]> {
        return await this.predictionRepository.find({
            order: {
                createdAt: 'DESC'
            }
        });
    }

    async getPredictionById(id: number): Promise<PredictionEntity | null> {
        return await this.predictionRepository.findOne({
            where: { id }
        });
    }

    async getPredictionByTrafficIntensities (trafficIntensity1: number, trafficIntensity2: number): Promise<PredictionEntity | null> {
        const delta = 15;
        return await this.predictionRepository.findOne({
            where: {  
                predictedTraffic1: Between(trafficIntensity1-delta, trafficIntensity1+delta),
                predictedTraffic2: Between(trafficIntensity2-delta, trafficIntensity2+delta)
            },
        });
    }

    async updatePrediction(id: number, predictionData: Partial<PredictionEntity>): Promise<PredictionEntity | null> {
        await this.predictionRepository.update(id, predictionData);
        return await this.getPredictionById(id);
    }

    async deletePrediction(id: number): Promise<boolean> {
        const result = await this.predictionRepository.delete(id);
        return result.affected !== 0;
    }

    async getLatestPrediction(): Promise<PredictionEntity | null> {
        return await this.predictionRepository.findOne({
            order: {
                createdAt: 'DESC'
            }
        });
    }
}