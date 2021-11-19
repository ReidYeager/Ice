[[#Diagram]]

## Platform
#### Responsibilities
- Window creation
- Memory allocation & destruction

## Rendering
#### Responsibilities
- Create an image out of input models and render settings
- Creating buffers and images

## Application
#### Responsibilities
- Material management
	- Define when & how it will render
- Model management
	- Load and parse the raw model
- Texture management
	- Load the raw image

## Game
The engine's client
#### Responsibilities
- Defining render settings
- Defining material settings
- Defining which models to render where

# Diagram
![[LayerSubsystemDiagram.png]]