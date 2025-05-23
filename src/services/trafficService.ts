import { AppDataSource } from '../datasource/datasource';
import { TrafficEntity } from '../entity/trafficEntity';
import { Repository } from 'typeorm';

export class TrafficService {
    private trafficRepository: Repository<TrafficEntity>;

    constructor() {
        this.trafficRepository = AppDataSource.getRepository(TrafficEntity);
    }

    async createTraffic(trafficData: Partial<TrafficEntity>): Promise<TrafficEntity> {
        const traffic = this.trafficRepository.create(trafficData);
        return await this.trafficRepository.save(traffic);
    }

    async getAllTraffic(): Promise<TrafficEntity[]> {
        return await this.trafficRepository.find({
            order: {
                createdAt: 'DESC'
            }
        });
    }

    async getTrafficById(id: number): Promise<TrafficEntity | null> {
        return await this.trafficRepository.findOne({
            where: { id }
        });
    }

    async updateTraffic(id: number, trafficData: Partial<TrafficEntity>): Promise<TrafficEntity | null> {
        await this.trafficRepository.update(id, trafficData);
        return await this.getTrafficById(id);
    }

    async deleteTraffic(id: number): Promise<boolean> {
        const result = await this.trafficRepository.delete(id);
        return result.affected !== 0;
    }

    async getLatestTraffic(): Promise<TrafficEntity | null> {
        return await this.trafficRepository.findOne({
            order: {
                createdAt: 'DESC'
            }
        });
    }
}