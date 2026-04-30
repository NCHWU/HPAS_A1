#include "PlotRenderer.h"

namespace PlotRenderer
{
	namespace {
        Shader* lineShader;
        Shader* debugLineShader;
        Shader* debugPointShader;
        Shader* pointShader2D;
        unsigned int lineVAO { 0 };
        unsigned int lineVBO { 0 };
        unsigned int pointVAO2D { 0 };
        unsigned int pointVBO2D { 0 };
        unsigned int labelsVBO { 0 };
        unsigned int debugLineVAO2D { 0 };
        unsigned int debugLineVBO2D { 0 };
        unsigned int debugPointVAO2D { 0 };
        unsigned int debugPointVBO2D { 0 };
        float debugLineSize2D { 2.0f };
        float debugPointSize2D { 10.0f };
        glm::vec2 size2D = glm::vec2(1.5f, 1.5f);

        // 2D movement
        float zoom { 1.0f };
        float moveSensitivity { 0.03f };
        float zoomSensitivity { 0.1f };
        glm::vec2 center { 0.5f };

        float padding { 0.05f };

        float referenceHeight = 800.0f;

        // Get the global scale factor for normalizing based on the window size
        float getScale()
        {
            return (float)WindowHandler::getWindowSize().y / referenceHeight;
        }

        // Update the minimum value of a vector (component-wise)
        void updateMin(glm::vec3& min, glm::vec3& val)
        {
            if (val.x < min.x)
                min.x = val.x;
            if (val.y < min.y)
                min.y = val.y;
            if (val.z < min.z)
                min.z = val.z;
        }
        // Update the minimum value of a vector (component-wise)
        void updateMin(vec& min, vec& val)
        {
            if (val.x < min.x)
                min.x = val.x;
            if (val.y < min.y)
                min.y = val.y;
        }

        // Update the maximum value of a vector (component-wise)
        void updateMax(glm::vec3& max, glm::vec3& val)
        {
            if (val.x > max.x)
                max.x = val.x;
            if (val.y > max.y)
                max.y = val.y;
            if (val.z > max.z)
                max.z = val.z;
        }
        // Update the maximum value of a vector (component-wise)
        void updateMax(vec& max, vec& val)
        {
            if (val.x > max.x)
                max.x = val.x;
            if (val.y > max.y)
                max.y = val.y;
        }

        // Calculate a "nice" interval size for a given range and number of intervals
        float niceInterval(float min, float max, int n)
        {
            // Calculate the rough size of each interval
            float range = max - min;
            float roughSize = range / n;

            // Calculate the magnitude of the rough size
            float magnitude = static_cast<float>(std::pow(10, std::floor(std::log10(roughSize))));

            // Normalize the rough size to the range [1, 10)
            float normalizedSize = roughSize / magnitude;

            // Choose a "nice" value from the set {1, 2, 5, 10}
            float niceSize;
            if (normalizedSize < 1.5) {
                niceSize = 1;
            } else if (normalizedSize < 3) {
                niceSize = 2;
            } else if (normalizedSize < 7) {
                niceSize = 5;
            } else {
                niceSize = 10;
            }

            // Return the adjusted interval size
            return niceSize * magnitude;
        }

        // Get the grid lines for a given range and size
        std::vector<GridLine> getLineValues(float min, float max, float size)
        {
            std::vector<GridLine> gridLines;

            int numLines = 6;

            float interval { niceInterval(min, max, numLines) };

            float start = min + (std::fmod(min, interval) > 0 ? interval - std::fmod(min, interval) : -std::fmod(min, interval));

            if (interval <= 0.0f) {
                // Invalid interval value
                return gridLines;
            }

            float currentY = start;

            while (currentY < max) {

                float worldPosition { ((currentY - min) / (max - min)) * size };
                float value { currentY };

                gridLines.push_back(GridLine { worldPosition, value });

                currentY += interval;
            }
            return gridLines;
        }

        void drawGrid2D(Camera& camera, glm::vec2 origin, glm::vec2 up, glm::vec2 right, float height, float width, std::vector<GridLine> verticalValues, std::vector<GridLine> horizontalValues)
        {
            std::vector<glm::vec3> points;

            // Adding horizontal lines
            for (const GridLine& gridLine : horizontalValues) {
                points.push_back(glm::vec3(origin + up * gridLine.worldHeight, 0.0f));
                points.push_back(glm::vec3(origin + up * gridLine.worldHeight + right * width, 0.0f));
            }

            // Adding vertical lines
            for (const GridLine& gridLine : verticalValues) {
                points.push_back(glm::vec3(origin + right * gridLine.worldHeight, 0.0f));
                points.push_back(glm::vec3(origin + right * gridLine.worldHeight + up * height, 0.0f));
            }

            // Rendering commands

            glLineWidth(1.0f * getScale());

            glBindVertexArray(lineVAO);

            glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
            glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

            lineShader->use();
            lineShader->setVector3("color", glm::vec3(0.5f));
            lineShader->setMat4("view", glm::mat4(1.0f));
            float aspectRatio = (float)WindowHandler::getWindowSize().x / (float)WindowHandler::getWindowSize().y;
            glm::mat4 aspectRatioCorrection = glm::mat4(1.0f);
            aspectRatioCorrection[0][0] = 1.0f / aspectRatio;
            lineShader->setMat4("projection", aspectRatioCorrection);

            glDrawArrays(GL_LINES, 0, (GLsizei)points.size());
            glBindVertexArray(0);
        }
	}

	void init()
	{
        lineShader = new Shader(RESOURCE_ROOT "shaders_source/lineVertex.shader", RESOURCE_ROOT "shaders_source/lineFragment.shader");
        debugLineShader = new Shader(RESOURCE_ROOT "shaders_source/debugLineVertex.shader", RESOURCE_ROOT "shaders_source/debugLineFragment.shader");
        debugPointShader = new Shader(RESOURCE_ROOT "shaders_source/debugPointVertex.shader", RESOURCE_ROOT "shaders_source/debugPointFragment.shader");
        pointShader2D = new Shader(RESOURCE_ROOT "shaders_source/pointVertex2D.shader", RESOURCE_ROOT "shaders_source/pointFragment2D.shader");


        glGenBuffers(1, &labelsVBO);

		glGenVertexArrays(1, &lineVAO);
        glBindVertexArray(lineVAO);

        glGenBuffers(1, &lineVBO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

        // 3 floats for 3 color channels
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, labelsVBO);
        glVertexAttribIPointer(1, 1, GL_INT, sizeof(int), (void*)0);
        glEnableVertexAttribArray(1);



        glGenVertexArrays(1, &pointVAO2D);
        glBindVertexArray(pointVAO2D);

        glGenBuffers(1, &pointVBO2D);
        glBindBuffer(GL_ARRAY_BUFFER, pointVBO2D);

#if TSNE_PRECISION == 1
        // 2 floats for 2D position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
#elif TSNE_PRECISION == 2
        // 2 doubles for 2D position
        glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
        glEnableVertexAttribArray(0);
#endif


        glBindBuffer(GL_ARRAY_BUFFER, labelsVBO);
        glVertexAttribIPointer(1, 1, GL_INT, sizeof(int), (void*)0);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);



        // Initialize the buffers for the 2D debug rendering

        glGenVertexArrays(1, &debugLineVAO2D);
        glBindVertexArray(debugLineVAO2D);

        glGenBuffers(1, &debugLineVBO2D);
        glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO2D);

        // 2 floats for 2D position, 3 floats for color
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        
        glGenVertexArrays(1, &debugPointVAO2D);
        glBindVertexArray(debugPointVAO2D);

        glGenBuffers(1, &debugPointVBO2D);
        glBindBuffer(GL_ARRAY_BUFFER, debugPointVBO2D);

        // 2 floats for 2D position, 3 floats for color
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    
	void drawPlot2D(Camera& camera)
    {
        std::vector<glm::vec3> points;

        // Building the viewable grid
        // (assuming looking into negative z, x is to the right, y is up)
        glm::vec3 bottomLeft { glm::vec3(-size2D.x / 2.0f, -size2D.y / 2.0f, 0.0f) };
        glm::vec3 bottomRight { glm::vec3(size2D.x / 2.0f, -size2D.y / 2.0f, 0.0f) };
        glm::vec3 topRight { glm::vec3(size2D.x / 2.0f, size2D.y / 2.0f, 0.0f) };
        glm::vec3 topLeft { glm::vec3(-size2D.x / 2.0f, size2D.y / 2.0f, 0.0f) };

        points.push_back(bottomLeft);
        points.push_back(topLeft);

        points.push_back(bottomLeft);
        points.push_back(bottomRight);

        // Rendering commands

        glLineWidth(2.0f * getScale());

        glBindVertexArray(lineVAO);

        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

        lineShader->use();
        lineShader->setVector3("color", glm::vec3(0.0f));
        lineShader->setMat4("view", glm::mat4(1.0f));
        float aspectRatio = (float)WindowHandler::getWindowSize().x / (float)WindowHandler::getWindowSize().y;
        glm::mat4 aspectRatioCorrection = glm::mat4(1.0f);
        aspectRatioCorrection[0][0] = 1.0f / aspectRatio;
        lineShader->setMat4("projection", aspectRatioCorrection);

        glDrawArrays(GL_LINES, 0, (GLsizei)points.size());
        glBindVertexArray(0);
    }

    void scatterPlot(Camera& camera, NDArray<vec> points, Settings& settings, DebugRenderData2D debugRenderData)
    {
        NDArray<int> labels;
        scatterPlot(camera, points, labels, settings, debugRenderData);
    }

    void scatterPlot(Camera& camera, NDArray<vec> points, NDArray<int>& labels, Settings& settings, DebugRenderData2D debugRenderData)
    {
        // Moving according to the latest camera inputs
        glm::vec2 movement = camera.getLastMove();
        center -= movement * moveSensitivity / zoom;
        zoom += ScrollManager::getScroll() * zoomSensitivity * zoom;

        bool hasLabels { labels.size() > 0 };

        glDisable(GL_DEPTH_TEST);
        drawPlot2D(camera);

        // Finding the boundaries of the data
        vec min = points(0);
        vec max = points(0);

        for (unsigned int i = 0; i < points.size(); i++) {
            updateMin(min, points(i));
            updateMax(max, points(i));
        }

        // Scale all dimensions equally
        bool keepDimensions { false };

        if (!settings.followData) {
            min = center - 0.5f / zoom;
            max = center + 0.5f / zoom;
        }

        glm::vec2 centerOffset = -(min + max) / (prec_float)2.0f;
        glm::vec2 dataSize = max - min;

        // Applying padding to the data
        min -= dataSize * padding;
        max += dataSize * padding;
        dataSize += 2 * padding * dataSize;
        glm::vec2 requiredScale = size2D / dataSize;
        glm::vec2 scalingFactor;

        if (keepDimensions) {
            scalingFactor = glm::vec2(std::min(requiredScale.x, requiredScale.y));
        } else {
            scalingFactor = requiredScale;
        }

        // Drawing the grids
        glm::vec2 X = glm::vec2(1.0f, 0.0f);
        glm::vec2 Y = glm::vec2(0.0f, 1.0f);
        glm::vec2 origin = glm::vec2(-size2D.x / 2.0f, -size2D.y / 2.0f);
        std::vector<GridLine> gridLinesX = getLineValues(min.x, max.x, size2D.x);
        std::vector<GridLine> gridLinesY = getLineValues(min.y, max.y, size2D.y);

        drawGrid2D(camera, origin, Y, X, size2D.y, size2D.x, gridLinesX, gridLinesY);

        float aspectRatio = (float)WindowHandler::getWindowSize().x / (float)WindowHandler::getWindowSize().y;
        glm::mat4 aspectRatioCorrection = glm::mat4(1.0f);
        aspectRatioCorrection[0][0] = 1.0f / aspectRatio;

        // Now drawing the text for the grid line values at the correct positions
        for (const GridLine& gridLine : gridLinesY) {
            Text text;
            text.setText(to_string_with_precision(gridLine.value, 2));

            // Calculating the necessary position
            glm::vec4 position = glm::vec4(origin + Y * gridLine.worldHeight, 0.0f, 1.0f);
            position = position * aspectRatioCorrection;

            text.setNormalizedPosition(glm::vec2(position.x, position.y));

            // Since this text is on the vertical axis (left of the axis), we need to set the origin to the bottom right
            text.setHorizontalOriginRight();
            text.setVerticalOriginBottom();

            // Applying the global scale according to the size of the window
            text.setScale(getScale());
            text.render();
        }

        for (const GridLine& gridLine : gridLinesX) {
            Text text;
            text.setText(to_string_with_precision(gridLine.value, 2));

            // Calculating the necessary position
            glm::vec4 position = glm::vec4(origin + X * gridLine.worldHeight, 0.0f, 1.0f);
            position = position * aspectRatioCorrection;
            text.setNormalizedPosition(glm::vec2(position.x, position.y));

            // Since this text is on the horizontal axis (bottom of the axis), we need to set the origin to the top left
            text.setHorizontalOriginLeft();
            text.setVerticalOriginTop();

            // Applying the global scale according to the size of the window
            text.setScale(getScale());
            text.render();
        }

	    glDisable(GL_PROGRAM_POINT_SIZE);

        glPointSize(5.0f * getScale());

        glBindVertexArray(pointVAO2D);

        glBindBuffer(GL_ARRAY_BUFFER, pointVBO2D);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vec), points.begin(), GL_STATIC_DRAW);

        if (hasLabels) {
            glBindBuffer(GL_ARRAY_BUFFER, labelsVBO);
            glBufferData(GL_ARRAY_BUFFER, labels.size() * sizeof(int), labels.begin(), GL_STATIC_DRAW);
        }

        pointShader2D->use();
        pointShader2D->setVector3("color", glm::vec3(1.0f, 0.0f, 0.0f));
        pointShader2D->setVector2("offset", glm::vec3(centerOffset, 0.0f));
        pointShader2D->setVector2("min", min);
        pointShader2D->setVector2("max", max);
        pointShader2D->setVector2("scalingFactor", glm::vec3(scalingFactor, 0.0f));
        pointShader2D->setMat4("view", glm::mat4(1.0f));
        pointShader2D->setMat4("projection", aspectRatioCorrection);
        pointShader2D->setBool("useLabel", settings.useLabelColors && hasLabels);

        glDrawArrays(GL_POINTS, 0, (GLsizei)points.size());
        glBindVertexArray(0);

        if (!settings.showDebug) {
            glEnable(GL_DEPTH_TEST);
            return;
        }


        // Drawing the debug data

        glLineWidth(debugLineSize2D * getScale());

        glBindVertexArray(debugLineVAO2D);

        glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO2D);
        glBufferData(GL_ARRAY_BUFFER, debugRenderData.lines.size() * sizeof(DebugRenderDataPoint2D), debugRenderData.lines.data(), GL_STATIC_DRAW);

        debugLineShader->use();
        debugLineShader->setVector3("color", glm::vec3(0.0f, 1.0f, 1.0f));
        debugLineShader->setMat4("view", glm::mat4(1.0f));
        debugLineShader->setMat4("projection", aspectRatioCorrection);
        debugLineShader->setVector3("offset", glm::vec3(centerOffset, 0.0f));
        debugLineShader->setVector3("scalingFactor", glm::vec3(scalingFactor, 0.0f));
        debugLineShader->setVector2("min", min);
        debugLineShader->setVector2("max", max);

        glDrawArrays(GL_LINES, 0, (GLsizei)debugRenderData.lines.size());

        // Debug points

        glPointSize(debugPointSize2D * getScale());

        glBindVertexArray(debugPointVAO2D);

        glBindBuffer(GL_ARRAY_BUFFER, debugPointVBO2D);
        glBufferData(GL_ARRAY_BUFFER, debugRenderData.points.size() * sizeof(DebugRenderDataPoint2D), debugRenderData.points.data(), GL_STATIC_DRAW);

        debugPointShader->use();
        debugPointShader->setVector3("color", glm::vec3(0.0f, 1.0f, 1.0f));
        debugPointShader->setMat4("view", glm::mat4(1.0f));
        debugPointShader->setMat4("projection", aspectRatioCorrection);
        debugPointShader->setVector3("offset", glm::vec3(centerOffset, 0.0f));
        debugPointShader->setVector3("scalingFactor", glm::vec3(scalingFactor, 0.0f));
        debugPointShader->setVector2("min", min);
        debugPointShader->setVector2("max", max);

        glDrawArrays(GL_POINTS, 0, (GLsizei)debugRenderData.points.size());

        glEnable(GL_DEPTH_TEST);
    }
}