interface FrameStats {
    fps: number;
    width: number;
    height: number;
    timestamp: Date;
    processingMode: string;
}

class EdgeDetectionViewer {
    private canvas: HTMLCanvasElement;
    private ctx: CanvasRenderingContext2D;
    private fpsElement: HTMLElement;
    private resolutionElement: HTMLElement;
    private modeElement: HTMLElement;
    private timestampElement: HTMLElement;
    private uploadBtn: HTMLElement;
    private sampleBtn: HTMLElement;
    private fileInput: HTMLInputElement;
    
    private currentStats: FrameStats;
    
    constructor() {
        this.canvas = document.getElementById('frameCanvas') as HTMLCanvasElement;
        this.ctx = this.canvas.getContext('2d')!;
        this.fpsElement = document.getElementById('fpsValue')!;
        this.resolutionElement = document.getElementById('resolutionValue')!;
        this.modeElement = document.getElementById('modeValue')!;
        this.timestampElement = document.getElementById('timestampValue')!;
        this.uploadBtn = document.getElementById('uploadBtn')!;
        this.sampleBtn = document.getElementById('loadSampleBtn')!;
        this.fileInput = document.getElementById('fileInput') as HTMLInputElement;
        
        this.currentStats = {
            fps: 0,
            width: 0,
            height: 0,
            timestamp: new Date(),
            processingMode: 'None'
        };
        
        this.setupEventListeners();
        this.loadSampleFrame();
    }
    
    private setupEventListeners(): void {
        this.uploadBtn.addEventListener('click', () => {
            this.fileInput.click();
        });
        
        this.fileInput.addEventListener('change', (e) => {
            const file = (e.target as HTMLInputElement).files?.[0];
            if (file) {
                this.loadImageFile(file);
            }
        });
        
        this.sampleBtn.addEventListener('click', () => {
            this.loadSampleFrame();
        });
        
        this.canvas.addEventListener('dragover', (e) => {
            e.preventDefault();
            this.canvas.style.border = '3px dashed #007bff';
        });
        
        this.canvas.addEventListener('dragleave', () => {
            this.canvas.style.border = '2px solid #ddd';
        });
        
        this.canvas.addEventListener('drop', (e) => {
            e.preventDefault();
            this.canvas.style.border = '2px solid #ddd';
            
            const file = e.dataTransfer?.files[0];
            if (file && file.type.startsWith('image/')) {
                this.loadImageFile(file);
            }
        });
    }
    
    private loadImageFile(file: File): void {
        const reader = new FileReader();
        
        reader.onload = (e) => {
            const img = new Image();
            img.onload = () => {
                this.displayFrame(img, {
                    fps: 15.0,
                    width: img.width,
                    height: img.height,
                    timestamp: new Date(),
                    processingMode: 'Edge Detection'
                });
            };
            img.src = e.target?.result as string;
        };
        
        reader.readAsDataURL(file);
    }
    
    private loadSampleFrame(): void {
        const sampleCanvas = document.createElement('canvas');
        sampleCanvas.width = 640;
        sampleCanvas.height = 480;
        const sampleCtx = sampleCanvas.getContext('2d')!;
        
        sampleCtx.fillStyle = '#000000';
        sampleCtx.fillRect(0, 0, 640, 480);
        
        sampleCtx.strokeStyle = '#FFFFFF';
        sampleCtx.lineWidth = 2;
        
        for (let i = 0; i < 10; i++) {
            sampleCtx.beginPath();
            const x = Math.random() * 640;
            const y = Math.random() * 480;
            const radius = Math.random() * 50 + 20;
            sampleCtx.arc(x, y, radius, 0, Math.PI * 2);
            sampleCtx.stroke();
        }
        
        for (let i = 0; i < 15; i++) {
            sampleCtx.beginPath();
            sampleCtx.moveTo(Math.random() * 640, Math.random() * 480);
            sampleCtx.lineTo(Math.random() * 640, Math.random() * 480);
            sampleCtx.stroke();
        }
        
        sampleCtx.fillStyle = '#FFFFFF';
        sampleCtx.font = 'bold 24px Arial';
        sampleCtx.fillText('SAMPLE EDGE DETECTION', 150, 240);
        
        const img = new Image();
        img.onload = () => {
            this.displayFrame(img, {
                fps: 15.0,
                width: 640,
                height: 480,
                timestamp: new Date(),
                processingMode: 'Edge Detection (Sample)'
            });
        };
        img.src = sampleCanvas.toDataURL();
    }
    
    private displayFrame(img: HTMLImageElement, stats: FrameStats): void {
        this.canvas.width = stats.width;
        this.canvas.height = stats.height;
        
        this.ctx.drawImage(img, 0, 0);
        
        this.currentStats = stats;
        this.updateStatsDisplay();
        
        console.log('Frame displayed:', stats);
    }
    
    private updateStatsDisplay(): void {
        this.fpsElement.textContent = this.currentStats.fps.toFixed(1);
        this.resolutionElement.textContent = 
            `${this.currentStats.width} × ${this.currentStats.height}`;
        this.modeElement.textContent = this.currentStats.processingMode;
        this.timestampElement.textContent = 
            this.currentStats.timestamp.toLocaleTimeString();
    }
    
    public updateFrameFromBase64(base64Data: string, stats: FrameStats): void {
        const img = new Image();
        img.onload = () => {
            this.displayFrame(img, stats);
        };
        img.src = `data:image/png;base64,${base64Data}`;
    }
    
    public exportFrame(): void {
        const dataUrl = this.canvas.toDataURL('image/png');
        const link = document.createElement('a');
        link.download = `frame_${Date.now()}.png`;
        link.href = dataUrl;
        link.click();
    }
}

document.addEventListener('DOMContentLoaded', () => {
    const viewer = new EdgeDetectionViewer();
    
    (window as any).viewer = viewer;
    
    console.log('Edge Detection Viewer initialized');
});