import { Entity, PrimaryGeneratedColumn, Column, CreateDateColumn, UpdateDateColumn } from 'typeorm';

@Entity("prediction")
export class PredictionEntity {
    @PrimaryGeneratedColumn()
    id: number;

    @Column()
    predictedTraffic1: number;

    @Column()
    predictedTraffic2: number;

    @Column()
    suggestedTiming1: number;

    @Column()
    suggestedTiming2: number;

    @CreateDateColumn()
    createdAt: Date;

}