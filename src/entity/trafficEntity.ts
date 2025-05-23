import { Entity, PrimaryGeneratedColumn, Column, CreateDateColumn } from 'typeorm';

@Entity("traffic")
export class TrafficEntity {
    @PrimaryGeneratedColumn()
    id: number;

    @Column()
    trafficIntensity1: number;

    @Column()
    trafficIntensity2: number;

    @Column()
    co2Level: number;

    @Column()
    lightLevel1: number;

    @Column()
    lightLevel2: number;

    @Column()
    currentGreenTime1: number;

    @Column()
    currentGreenTime2: number;

    @Column()
    pedestrianRequest1: boolean;

    @Column()
    pedestrianRequest2: boolean;

    @CreateDateColumn()
    createdAt: Date;
}